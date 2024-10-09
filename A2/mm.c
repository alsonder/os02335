#include <stdint.h>
#include <stdio.h>
#include "mm.h"

//Used GitHub CoPilot plugin for VSCode and http://perplexity.ai for a lot of bugfixing and refactoring

/* Proposed data structure elements */
typedef struct header {
    struct header *next;      // Bit 0 is used to indicate a free block
    uint64_t user_block[0];   // Empty array to ensure alignment of user block
} BlockHeader;

// Macros to access fields in the header

#define FREE_MASK 0x1 // Mask to extract the free bit
#define GET_NEXT(p) ((BlockHeader *)((uintptr_t)(p->next) & ~FREE_MASK)) // Mask out the free bit
#define SET_NEXT(p, n) p->next = (BlockHeader *)((uintptr_t)(n) | ((uintptr_t)(p->next) & FREE_MASK)) // Preserve the free bit
#define GET_FREE(p) (uint8_t)((uintptr_t)(p->next) & FREE_MASK) // Extract the free bit
#define SET_FREE(p, f) p->next = (BlockHeader *)((uintptr_t)GET_NEXT(p) | (f & FREE_MASK)) // Set the free bit
#define SIZE(p) ((uintptr_t)GET_NEXT(p) - (uintptr_t)(p) - sizeof(BlockHeader)) // Calculate the size of the block
#define MIN_SIZE (8)  // Minimum size of a block (excluding header)

static BlockHeader *first = NULL;
static BlockHeader *current = NULL;

extern const uintptr_t memory_start;
extern const uintptr_t memory_end;

/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 */
void simple_init() {
    uintptr_t aligned_memory_start = (memory_start + 7) & ~7;  // Align to 8-byte boundary
    uintptr_t aligned_memory_end = memory_end & ~7;             // Align to 8-byte boundary

    if (first == NULL) {
        // Ensure there is enough space for the initial blocks
        if (aligned_memory_start + 2 * sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
            // Create the first free block
            first = (BlockHeader *)aligned_memory_start;
            first->next = (BlockHeader *)(aligned_memory_end - sizeof(BlockHeader));  // Set the next to the end block
            SET_FREE(first, 1);  // Mark as free

            // Create the last block as a sentinel
            BlockHeader *last = GET_NEXT(first);
            last->next = NULL;  // End of the list
            SET_FREE(last, 0);  // Mark as allocated (end marker)

            // Set the current pointer for next-fit strategy
            current = first;
        }
    }
}

/**
 * @name    simple_malloc
 * @brief   Allocate at least `size` contiguous bytes of memory and return a pointer to the first byte.
 * @param   size_t size Number of bytes to allocate.
 * @retval  Pointer to the start of the allocated memory or NULL if not possible.
 */
void *simple_malloc(size_t size) {
    if (first == NULL) {
        simple_init();  // Initialize memory if not done already
        if (first == NULL) return NULL;
    }

    // Align the requested size to a multiple of 8 bytes
    size_t aligned_size = (size + 7) & ~7;

    // Start searching for a suitable free block using the next-fit strategy
    BlockHeader *search_start = current;
    do {
        if (GET_FREE(current)) {
            size_t current_block_size = SIZE(current);

            // Check if the current free block is large enough for the requested size
            if (current_block_size >= aligned_size) {
                // If the remainder of the block is too small, use the entire block
                if (current_block_size - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
                    SET_FREE(current, 0);  // Mark the entire block as allocated
                } else {
                    // Split the block: create a new block with the remaining space
                    BlockHeader *new_block = (BlockHeader *)((uintptr_t)current + sizeof(BlockHeader) + aligned_size);
                    new_block->next = GET_NEXT(current);  // Link new block to the next block
                    SET_FREE(new_block, 1);               // Mark new block as free

                    // Update the current block's next pointer and mark it as allocated
                    SET_NEXT(current, new_block);
                    SET_FREE(current, 0);
                }

                // Save the pointer to the user memory region
                void *user_pointer = (void *)(current->user_block);

                // Move the `current` pointer to the next block for the next-fit strategy
                current = GET_NEXT(current);

                return user_pointer;  // Return the address of the user block (after the header)
            }
        }

        // Move to the next block
        current = GET_NEXT(current);
    } while (current != search_start);  // Stop if we come back to the starting block

    // No suitable block found
    return NULL;
}

/**
 * @name    simple_free
 * @brief   Frees previously allocated memory and makes it available for subsequent calls to simple_malloc
 * @param   void *ptr Pointer to the memory to free.
 */
void simple_free(void *ptr) {
    if (ptr == NULL) return;

    // Find the block header corresponding to the user pointer
    BlockHeader *block_to_free = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));

    // If the block is already free, print a warning
    if (GET_FREE(block_to_free)) {
        printf("Warning: Attempting to free an already free block at address %p.\n", ptr);
        return;
    }

    // Mark the block as free
    SET_FREE(block_to_free, 1);

    // Coalesce with the next block if it is free
    BlockHeader *next_block = GET_NEXT(block_to_free);
    if (GET_FREE(next_block) && next_block != block_to_free) {  // Ensure we are not merging with itself
        // Merge current block with the next free block
        SET_NEXT(block_to_free, GET_NEXT(next_block));
    }

    // Coalesce with the previous block if it is free
    BlockHeader *prev_block = first;
    while (prev_block != NULL && GET_NEXT(prev_block) != block_to_free) {
        prev_block = GET_NEXT(prev_block);
    }
    if (prev_block != NULL && GET_FREE(prev_block) && prev_block != block_to_free) {  // Ensure valid previous block
        // Merge previous block with the current free block
        SET_NEXT(prev_block, GET_NEXT(block_to_free));
        block_to_free = prev_block;  // Update block_to_free to the merged block
    }

    // Update the `current` pointer to this block to optimize the next-fit strategy
    current = block_to_free;
}
