#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* DISK CONFIGURATION */
#define DISK_SIZE 102400   
#define BLOCK_SIZE 1024    
#define MAX_FILES 32      
#define DISK_NAME "dysk_wirtualny.dsk"

/* DATA STRUCTURES */

typedef struct {
    char name[32];      
    int start_block;    
    int size_bytes;     
    int used;           
} dir_entry_t;

typedef struct {
    int total_size;
    int block_count;
    dir_entry_t directory[MAX_FILES];
} super_block_t;

/* GLOBAL VARIABLES */
FILE *disk_ptr;
super_block_t sb;
int data_start_offset = sizeof(super_block_t);

/* SUPPORT FUNCTIONS */

void open_disk(const char *mode) {
    disk_ptr = fopen(DISK_NAME, mode);
    if (!disk_ptr) {
        perror("ERROR: unable to open disk");
        exit(1);
    }
}

void load_sb() {
    open_disk("r+b");
    if (fread(&sb, sizeof(super_block_t), 1, disk_ptr) != 1) {
        printf("ERROR: Cannot read super block\n");
        exit(1);
    }
}

void save_sb() {
    fseek(disk_ptr, 0, SEEK_SET);
    fwrite(&sb, sizeof(super_block_t), 1, disk_ptr);
}


int needed_blocks(int bytes) {
    return (bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

/* MAIN FUNCTIONS */

void create_disk() {
    int i, data_area_size;
    char zero_buff[BLOCK_SIZE] = {0};
    disk_ptr = fopen(DISK_NAME, "wb");
    if (!disk_ptr) { perror("Creation error"); exit(1); }

    
    sb.total_size = DISK_SIZE;
    
    data_area_size = DISK_SIZE - sizeof(super_block_t);
    sb.block_count = data_area_size / BLOCK_SIZE;

    for(i=0; i<MAX_FILES; i++) sb.directory[i].used = 0;

    fwrite(&sb, sizeof(super_block_t), 1, disk_ptr);
    
    for(i=0; i<sb.block_count; i++) {
        fwrite(zero_buff, 1, BLOCK_SIZE, disk_ptr);
    }
    
    printf("Created disk: %s, Size: %d bytes, Blocks: %d\n", DISK_NAME, DISK_SIZE, sb.block_count);
    fclose(disk_ptr);
}

void remove_disk() {
    if (remove(DISK_NAME) == 0) printf("Virtual disk removed\n");
    else perror("Cannot remove virtual disk");
}


void copy_to_virtual(char *src_filename, char *dest_name) {
    int i, k, dir_idx;
    int file_size, blocks_needed;
    int start, len;
    int start_block, current_streak;
    FILE *src;
    char *buffer, *map;
    
    load_sb();

    for(i=0; i<MAX_FILES; i++) {
        if(sb.directory[i].used && strcmp(sb.directory[i].name, dest_name) == 0) {
            printf("ERROR: File '%s' already exists.\n", dest_name);
            fclose(disk_ptr);
            return;
        }
    }

    src = fopen(src_filename, "rb");
    if(!src) { perror("No source file"); fclose(disk_ptr); return; }

    fseek(src, 0, SEEK_END);
    file_size = ftell(src);
    fseek(src, 0, SEEK_SET);
    blocks_needed = needed_blocks(file_size);

    dir_idx = -1;
    for(i=0; i<MAX_FILES; i++) {
        if(!sb.directory[i].used) { dir_idx = i; break; }
    }
    if(dir_idx == -1) { printf("ERROR: Catalogue full!\n"); fclose(src); fclose(disk_ptr); return; }

    map = calloc(sb.block_count, sizeof(char));
    for(i=0; i<MAX_FILES; i++) {
        if(sb.directory[i].used) {
            start = sb.directory[i].start_block;
            len = needed_blocks(sb.directory[i].size_bytes);
            for(k=0; k<len; k++) map[start + k] = 1;
        }
    }

    start_block = -1;
    current_streak = 0;
    
    for(i=0; i<sb.block_count; i++) {
        if(map[i] == 0) {
            current_streak++;
            if(current_streak == blocks_needed) {
                start_block = i - blocks_needed + 1;
                break;
            }
        } else {
            current_streak = 0;
        }
    }
    free(map);

    if(start_block == -1) {
        printf("ERROR: No contiguous space (internal fragmentation) or no space at all.\n");
        printf("Required blocks: %d\n", blocks_needed);
        fclose(src);
        fclose(disk_ptr);
        return;
    }

    
    buffer = calloc(blocks_needed, BLOCK_SIZE);
    fread(buffer, 1, file_size, src);
    
    fseek(disk_ptr, data_start_offset + (start_block * BLOCK_SIZE), SEEK_SET);
    fwrite(buffer, 1, blocks_needed * BLOCK_SIZE, disk_ptr);
    free(buffer);

    strncpy(sb.directory[dir_idx].name, dest_name, 31);
    sb.directory[dir_idx].start_block = start_block;
    sb.directory[dir_idx].size_bytes = file_size;
    sb.directory[dir_idx].used = 1;

    save_sb();
    fclose(src);
    fclose(disk_ptr);
    printf("Saved file '%s' (%d B) in block %d.\n", dest_name, file_size, start_block);
}

void copy_from_virtual(char *src_name, char *dest_filename) {
    int idx = -1;
    int i;
    FILE *dest;
    char *buffer;
    
    load_sb();
    for(i=0; i<MAX_FILES; i++) {
        if(sb.directory[i].used && strcmp(sb.directory[i].name, src_name) == 0) {
            idx = i; break;
        }
    }

    if(idx == -1) { printf("File not found: '%s'\n", src_name); fclose(disk_ptr); return; }

    dest = fopen(dest_filename, "wb");
    if(!dest) { perror("Failed to save on local disk"); fclose(disk_ptr); return; }

    buffer = malloc(sb.directory[idx].size_bytes);
    fseek(disk_ptr, data_start_offset + (sb.directory[idx].start_block * BLOCK_SIZE), SEEK_SET);
    fread(buffer, 1, sb.directory[idx].size_bytes, disk_ptr);
    fwrite(buffer, 1, sb.directory[idx].size_bytes, dest);

    free(buffer);
    fclose(dest);
    fclose(disk_ptr);
    printf("Copied '%s' onto Minix's disk.\n", src_name);
}

void delete_file(char *name) {
    int i;
    load_sb();
    for(i=0; i<MAX_FILES; i++) {
        if(sb.directory[i].used && strcmp(sb.directory[i].name, name) == 0) {
            sb.directory[i].used = 0;
            save_sb();
            fclose(disk_ptr);
            printf("File deleted '%s'.\n", name);
            return;
        }
    }
    printf("File not found.\n");
    fclose(disk_ptr);
}

void list_dir() {
    int i;
    load_sb();
    printf("\n--- CATALOGUE ---\n");
    printf("%-15s %-10s %-10s\n", "Name", "Size", "Starting block");
    for(i=0; i<MAX_FILES; i++) {
        if(sb.directory[i].used) {
            printf("%-15s %-10d %-10d\n", 
                sb.directory[i].name, 
                sb.directory[i].size_bytes, 
                sb.directory[i].start_block);
        }
    }
    fclose(disk_ptr);
}

int compare_entries(const void *a, const void *b) {
    dir_entry_t *entryA = (dir_entry_t *)a;
    dir_entry_t *entryB = (dir_entry_t *)b;
    return (entryA->start_block - entryB->start_block);
}

void show_map() {
    int i, count;
    int current_block, free_blocks, blocks, internal_frag;
    dir_entry_t sorted[MAX_FILES];
    load_sb();
    
    count = 0;
    for(i=0; i<MAX_FILES; i++) {
        if(sb.directory[i].used) sorted[count++] = sb.directory[i];
    }
    
    qsort(sorted, count, sizeof(dir_entry_t), compare_entries);

    printf("\n--- MAP OF SPACE IN DISK (Block = %d B) ---\n", BLOCK_SIZE);
    printf("ADDRESS (BLOCK)\tSTATE\t\tSIZE (BLOCKS)\tDESC\n");
    printf("------------------------------------------------------------\n");

    current_block = 0;
    for(i=0; i<count; i++) {
        if(sorted[i].start_block > current_block) {
            free_blocks = sorted[i].start_block - current_block;
            printf("%-12d\tFREE\t\t%d\t\t[ --- ]\n", current_block, free_blocks);
        }

        blocks = needed_blocks(sorted[i].size_bytes);
        internal_frag = (blocks * BLOCK_SIZE) - sorted[i].size_bytes;
        
        printf("%-12d\tOCCUPIED\t\t%d\t\tFILE: %s (Inter. frag.: %d B)\n", 
               sorted[i].start_block, blocks, sorted[i].name, internal_frag);
        
        current_block = sorted[i].start_block + blocks;
    }

    if(current_block < sb.block_count) {
        printf("%-12d\tFREE\t\t%d\t\t[ --- ]\n", current_block, sb.block_count - current_block);
    }
    printf("------------------------------------------------------------\n");
    printf("Blocks sum: %d\n", sb.block_count);

    fclose(disk_ptr);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [command] [arg...]\n", argv[0]);
        printf("  -c            : Create virtual disk\n");
        printf("  -cp <src> <dst> : Copy to virtual disk\n");
        printf("  -out <src> <dst>: Copy from virtual disk\n");
        printf("  -ls           : File list\n");
        printf("  -rm <name>    : Delete file\n");
        printf("  -rmdisk       : Remove virtual disk\n");
        printf("  -map          : Show disk map\n");
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0) create_disk();
    else if (strcmp(argv[1], "-rmdisk") == 0) remove_disk();
    else if (strcmp(argv[1], "-ls") == 0) list_dir();
    else if (strcmp(argv[1], "-map") == 0) show_map();
    else if (strcmp(argv[1], "-rm") == 0 && argc == 3) delete_file(argv[2]);
    else if (strcmp(argv[1], "-cp") == 0 && argc == 4) copy_to_virtual(argv[2], argv[3]);
    else if (strcmp(argv[1], "-out") == 0 && argc == 4) copy_from_virtual(argv[2], argv[3]);
    else printf("Undefined command or wrong arguments.\n");

    return 0;
}

