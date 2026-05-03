/*
 * ============================================================
 * LODGING MANAGEMENT SYSTEM
 * CC104: Data Structures and Algorithms - Final Project
 * ============================================================
 *
 * DATA STRUCTURES USED:
 *   1. Array         - Stores all rooms (Room rooms[MAX_ROOMS])
 *   2. Linked List   - Guest waitlist (queue of guests waiting for a room)
 *   3. Hash Table    - Fast room lookup by room number
 *
 * ALGORITHMS USED:
 *   1. Bubble Sort   - Sort rooms by price or room number
 *   2. Linear Search - Search for available rooms by type/price
 *
 * EXTERNAL REFERENCE (with adaptation):
 *   The double hashing technique used in hashFunction() and hashProbe()
 *   below is adapted from:
 *     James Routley, "Write a Hash Table in C"
 *     GitHub: https://github.com/jamesroutley/write-a-hash-table
 *     License: MIT
 *   Original uses string keys and open addressing. This project adapts
 *   the two-hash formula to integer room numbers with separate chaining.
 *   Prompt used: "adapt double hash function for integer keys in C"
 *   Adaptation: replaced string hashing (djb2) with integer modulo,
 *               applied to chaining instead of open addressing.
 *
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * CONSTANTS
 * ============================================================ */
#define MAX_ROOMS     50
#define MAX_NAME      50
#define HASH_SIZE     100
#define ROOM_TYPES    3  /* Standard, Deluxe, Suite */

/* ============================================================
 * DATA STRUCTURE 1: ARRAY - Room Storage
 * All rooms are stored in a fixed-size array for O(1) access by index.
 * ============================================================ */
typedef struct {
    int    roomNumber;           /* Unique room ID */
    char   type[20];             /* Standard, Deluxe, Suite */
    float  pricePerNight;
    int    isOccupied;           /* 0 = available, 1 = occupied */
    char   guestName[MAX_NAME];  /* Current guest (if occupied) */
    int    nights;               /* Number of nights booked */
} Room;

/* ============================================================
 * DATA STRUCTURE 2: LINKED LIST - Guest Waitlist
 * Guests who want a room that's fully booked go into a waitlist.
 * When a room is freed, the first guest on the list gets it.
 * ============================================================ */
typedef struct WaitlistNode {
    char              guestName[MAX_NAME];
    char              roomType[20];
    struct WaitlistNode *next;
} WaitlistNode;

typedef struct {
    WaitlistNode *head;
    WaitlistNode *tail;
    int           count;
} Waitlist;

/* ============================================================
 * DATA STRUCTURE 3: HASH TABLE - Fast Room Lookup
 * Maps room numbers to their array index for O(1) average lookup.
 * Collision handled by chaining (linked list per bucket).
 * ============================================================ */
typedef struct HashNode {
    int            roomNumber;
    int            arrayIndex;  /* Index in the rooms[] array */
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode *buckets[HASH_SIZE];
} HashTable;

/* ============================================================
 * GLOBAL DATA
 * ============================================================ */
Room      rooms[MAX_ROOMS];
int       roomCount = 0;
Waitlist  waitlist  = {NULL, NULL, 0};
HashTable hashTable;

/* ============================================================
 * HASH TABLE FUNCTIONS
 *
 * Double hashing technique adapted from:
 *   James Routley, "Write a Hash Table in C"
 *   https://github.com/jamesroutley/write-a-hash-table (MIT License)
 *
 * Original implementation uses string keys with djb2 + a second prime-based
 * hash for open addressing. Here we adapt the two-hash formula for integer
 * room numbers with separate chaining (linked list per bucket).
 *
 * Formula: bucket = (hash1 + attempt * hash2) % TABLE_SIZE
 *   hash1 = roomNumber % HASH_SIZE          (primary bucket)
 *   hash2 = 1 + (roomNumber % (HASH_SIZE-1)) (secondary offset, always > 0)
 *
 * For our chaining implementation, attempt is always 0 (one bucket per key),
 * so the effective bucket is just hash1. The hash2 function is retained for
 * demonstration of the double-hash concept and for future open-addressing use.
 * ============================================================ */

/* Primary hash: maps room number to its main bucket */
int hashFunction(int roomNumber) {
    return roomNumber % HASH_SIZE;
}

/*
 * Secondary hash (from double-hashing pattern):
 * Ensures a non-zero stride so probing never cycles back too early.
 * Adapted from jamesroutley/write-a-hash-table (MIT).
 */
int hashFunction2(int roomNumber) {
    return 1 + (roomNumber % (HASH_SIZE - 1));
}

/*
 * Combined probe function — computes the bucket for a given attempt.
 * attempt=0 gives the primary bucket (same as hashFunction).
 * This matches the double-hash formula from the GitHub reference above.
 */
int hashProbe(int roomNumber, int attempt) {
    return (hashFunction(roomNumber) + attempt * hashFunction2(roomNumber)) % HASH_SIZE;
}

/* Insert a room number -> array index mapping into the hash table */
void hashInsert(int roomNumber, int arrayIndex) {
    int bucket = hashProbe(roomNumber, 0); /* attempt=0 => primary bucket */

    HashNode *newNode = (HashNode *)malloc(sizeof(HashNode));
    if (!newNode) { printf("Memory error.\n"); return; }

    newNode->roomNumber  = roomNumber;
    newNode->arrayIndex  = arrayIndex;
    newNode->next        = hashTable.buckets[bucket];
    hashTable.buckets[bucket] = newNode;
}

/* Look up a room by room number; returns array index or -1 if not found */
int hashLookup(int roomNumber) {
    int bucket = hashProbe(roomNumber, 0); /* attempt=0 => primary bucket */
    HashNode *curr = hashTable.buckets[bucket];

    while (curr) {
        if (curr->roomNumber == roomNumber)
            return curr->arrayIndex;
        curr = curr->next;
    }
    return -1; /* Not found */
}

/* Initialize the hash table (set all buckets to NULL) */
void initHashTable() {
    for (int i = 0; i < HASH_SIZE; i++)
        hashTable.buckets[i] = NULL;
}

/* ============================================================
 * LINKED LIST (WAITLIST) FUNCTIONS
 * ============================================================ */

/* Add a guest to the end of the waitlist */
void addToWaitlist(const char *guestName, const char *roomType) {
    WaitlistNode *node = (WaitlistNode *)malloc(sizeof(WaitlistNode));
    if (!node) { printf("Memory error.\n"); return; }

    strncpy(node->guestName, guestName, MAX_NAME - 1);
    strncpy(node->roomType,  roomType,  19);
    node->next = NULL;

    /* Append to tail for FIFO (queue) behavior */
    if (!waitlist.tail) {
        waitlist.head = waitlist.tail = node;
    } else {
        waitlist.tail->next = node;
        waitlist.tail       = node;
    }
    waitlist.count++;
    printf("  [Waitlist] %s added to waitlist for room type: %s\n",
           guestName, roomType);
}

/* Remove and return the first guest from the waitlist */
WaitlistNode *dequeueWaitlist() {
    if (!waitlist.head) return NULL;

    WaitlistNode *node = waitlist.head;
    waitlist.head = waitlist.head->next;
    if (!waitlist.head) waitlist.tail = NULL;
    waitlist.count--;
    node->next = NULL;
    return node;
}

/* Display all guests currently on the waitlist */
void showWaitlist() {
    if (!waitlist.head) {
        printf("  Waitlist is empty.\n");
        return;
    }
    WaitlistNode *curr = waitlist.head;
    int pos = 1;
    while (curr) {
        printf("  %d. %s (wants: %s)\n", pos++, curr->guestName, curr->roomType);
        curr = curr->next;
    }
}

/* ============================================================
 * ALGORITHM 1: BUBBLE SORT
 * Sorts the rooms[] array. Mode 0 = by room number, Mode 1 = by price.
 * Time complexity: O(n^2) — simple and easy to understand for a student project.
 * ============================================================ */
void bubbleSort(int mode) {
    /* Outer pass: n-1 passes needed */
    for (int i = 0; i < roomCount - 1; i++) {
        for (int j = 0; j < roomCount - i - 1; j++) {
            int shouldSwap = 0;

            if (mode == 0 && rooms[j].roomNumber > rooms[j+1].roomNumber)
                shouldSwap = 1;
            else if (mode == 1 && rooms[j].pricePerNight > rooms[j+1].pricePerNight)
                shouldSwap = 1;

            if (shouldSwap) {
                /* Swap rooms[j] and rooms[j+1] */
                Room temp  = rooms[j];
                rooms[j]   = rooms[j+1];
                rooms[j+1] = temp;
            }
        }
    }
    /* After sorting, re-build the hash table so indexes stay correct */
    initHashTable();
    for (int i = 0; i < roomCount; i++)
        hashInsert(rooms[i].roomNumber, i);
}

/* ============================================================
 * ALGORITHM 2: LINEAR SEARCH
 * Searches through rooms[] for available rooms of a given type.
 * Returns the first match index, or -1 if none found.
 * Time complexity: O(n)
 * ============================================================ */
int linearSearchAvailable(const char *type) {
    for (int i = 0; i < roomCount; i++) {
        /* Check both conditions: correct type AND not occupied */
        if (strcmp(rooms[i].type, type) == 0 && rooms[i].isOccupied == 0)
            return i;
    }
    return -1; /* No available room of that type */
}

/* ============================================================
 * ROOM MANAGEMENT FUNCTIONS
 * ============================================================ */

/* Add a new room to the system */
void addRoom(int roomNumber, const char *type, float price) {
    if (roomCount >= MAX_ROOMS) {
        printf("  Error: Room capacity full.\n");
        return;
    }
    /* Check for duplicate room number using hash table */
    if (hashLookup(roomNumber) != -1) {
        printf("  Error: Room %d already exists.\n", roomNumber);
        return;
    }

    /* Populate the room struct */
    rooms[roomCount].roomNumber    = roomNumber;
    rooms[roomCount].pricePerNight = price;
    rooms[roomCount].isOccupied    = 0;
    rooms[roomCount].nights        = 0;
    strncpy(rooms[roomCount].type,      type, 19);
    strncpy(rooms[roomCount].guestName, "",   MAX_NAME - 1);

    /* Register in hash table for fast lookup */
    hashInsert(roomNumber, roomCount);

    roomCount++;
    printf("  Room %d (%s) added at P%.2f/night.\n", roomNumber, type, price);
}

/* Check in a guest to a specific room number */
void checkIn(int roomNumber, const char *guestName, int nights) {
    int idx = hashLookup(roomNumber); /* O(1) average via hash table */

    if (idx == -1) {
        printf("  Error: Room %d not found.\n", roomNumber);
        return;
    }
    if (rooms[idx].isOccupied) {
        printf("  Room %d is already occupied by %s.\n",
               roomNumber, rooms[idx].guestName);
        return;
    }

    rooms[idx].isOccupied = 1;
    rooms[idx].nights     = nights;
    strncpy(rooms[idx].guestName, guestName, MAX_NAME - 1);

    printf("  Check-in successful! %s is in Room %d for %d night(s). Total: P%.2f\n",
           guestName, roomNumber, nights,
           nights * rooms[idx].pricePerNight);
}

/* Check out a guest from a room */
void checkOut(int roomNumber) {
    int idx = hashLookup(roomNumber);

    if (idx == -1) {
        printf("  Error: Room %d not found.\n", roomNumber);
        return;
    }
    if (!rooms[idx].isOccupied) {
        printf("  Room %d is already vacant.\n", roomNumber);
        return;
    }

    float total = rooms[idx].nights * rooms[idx].pricePerNight;
    printf("  Checkout: %s from Room %d. Total charged: P%.2f\n",
           rooms[idx].guestName, roomNumber, total);

    /* Clear guest info */
    rooms[idx].isOccupied = 0;
    rooms[idx].nights     = 0;
    strncpy(rooms[idx].guestName, "", MAX_NAME - 1);

    /* Check if someone on the waitlist wants this room type */
    if (waitlist.count > 0) {
        WaitlistNode *next = waitlist.head;
        if (strcmp(next->roomType, rooms[idx].type) == 0) {
            printf("  [Waitlist] Notifying %s - Room %d (%s) is now available!\n",
                   next->guestName, roomNumber, rooms[idx].type);
            dequeueWaitlist();
            free(next);
        }
    }
}

/* Display all rooms */
void displayRooms() {
    printf("\n  %-6s %-12s %-12s %-10s %s\n",
           "Room", "Type", "Price/Night", "Status", "Guest");
    printf("  %-6s %-12s %-12s %-10s %s\n",
           "------", "------------", "------------", "----------", "-----");

    for (int i = 0; i < roomCount; i++) {
        printf("  %-6d %-12s P%-11.2f %-10s %s\n",
               rooms[i].roomNumber,
               rooms[i].type,
               rooms[i].pricePerNight,
               rooms[i].isOccupied ? "Occupied" : "Available",
               rooms[i].isOccupied ? rooms[i].guestName : "-");
    }
}

/* Search for available rooms by type using linear search */
void searchAvailableByType(const char *type) {
    printf("\n  Available rooms of type '%s':\n", type);
    int found = 0;

    for (int i = 0; i < roomCount; i++) {
        if (strcmp(rooms[i].type, type) == 0 && !rooms[i].isOccupied) {
            printf("    Room %d - P%.2f/night\n",
                   rooms[i].roomNumber, rooms[i].pricePerNight);
            found++;
        }
    }
    if (!found) printf("    No available rooms of that type.\n");
}

/* ============================================================
 * SAMPLE DATA - pre-loaded rooms for demonstration
 * ============================================================ */
void loadSampleData() {
    addRoom(101, "Standard", 800.00);
    addRoom(102, "Standard", 800.00);
    addRoom(103, "Standard", 850.00);
    addRoom(201, "Deluxe",   1500.00);
    addRoom(202, "Deluxe",   1500.00);
    addRoom(203, "Deluxe",   1600.00);
    addRoom(301, "Suite",    3000.00);
    addRoom(302, "Suite",    3200.00);
}

/* ============================================================
 * MAIN MENU
 * ============================================================ */
int main() {
    initHashTable();
    loadSampleData();

    int choice;

    do {
        printf("\n");
        printf("  ====================================\n");
        printf("    LODGING MANAGEMENT SYSTEM\n");
        printf("  ====================================\n");
        printf("  1. View All Rooms\n");
        printf("  2. Check In Guest\n");
        printf("  3. Check Out Guest\n");
        printf("  4. Add to Waitlist\n");
        printf("  5. View Waitlist\n");
        printf("  6. Search Available Rooms by Type\n");
        printf("  7. Sort Rooms by Price\n");
        printf("  8. Sort Rooms by Room Number\n");
        printf("  9. Add New Room\n");
        printf("  0. Exit\n");
        printf("  ------------------------------------\n");
        printf("  Choice: ");
        scanf("%d", &choice);

        char name[MAX_NAME], type[20];
        int  roomNum, nights;
        float price;

        switch (choice) {
            case 1:
                displayRooms();
                break;

            case 2:
                printf("  Room number: ");  scanf("%d", &roomNum);
                printf("  Guest name: ");   scanf(" %[^\n]", name);
                printf("  Nights: ");       scanf("%d", &nights);
                checkIn(roomNum, name, nights);
                break;

            case 3:
                printf("  Room number: "); scanf("%d", &roomNum);
                checkOut(roomNum);
                break;

            case 4:
                printf("  Guest name: ");  scanf(" %[^\n]", name);
                printf("  Room type (Standard/Deluxe/Suite): ");
                scanf("%s", type);
                addToWaitlist(name, type);
                break;

            case 5:
                printf("\n  === WAITLIST ===\n");
                showWaitlist();
                break;

            case 6:
                printf("  Room type to search (Standard/Deluxe/Suite): ");
                scanf("%s", type);
                searchAvailableByType(type);
                break;

            case 7:
                bubbleSort(1);
                printf("  Rooms sorted by price.\n");
                displayRooms();
                break;

            case 8:
                bubbleSort(0);
                printf("  Rooms sorted by room number.\n");
                displayRooms();
                break;

            case 9:
                printf("  Room number: "); scanf("%d", &roomNum);
                printf("  Type (Standard/Deluxe/Suite): "); scanf("%s", type);
                printf("  Price per night: "); scanf("%f", &price);
                addRoom(roomNum, type, price);
                break;

            case 0:
                printf("  Goodbye!\n");
                break;

            default:
                printf("  Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}
