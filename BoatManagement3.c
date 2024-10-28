#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define NAME_LEN 127

typedef enum {
    slip,
    land,
    trailor,
    storage,
    no_place
} PlaceType;

typedef union {
    int slip_num;
    char bay_letter;
    char trailor_license[10];
    int storage_space;
} ExtraInfo;

typedef struct {
    char name[NAME_LEN + 1];
    float length;
    PlaceType place;
    ExtraInfo extra;
    float amount_owed;
} Boat;

Boat *boats[MAX_BOATS] = {NULL};
int num_boats = 0;

// Helper functions
PlaceType StringToPlaceType(char *PlaceString) {
    if (!strcasecmp(PlaceString, "slip")) return slip;
    if (!strcasecmp(PlaceString, "land")) return land;
    if (!strcasecmp(PlaceString, "trailor")) return trailor;
    if (!strcasecmp(PlaceString, "storage")) return storage;
    return no_place;
}

char *PlaceToString(PlaceType place) {
    switch (place) {
        case slip: return "slip";
        case land: return "land";
        case trailor: return "trailor";
        case storage: return "storage";
        case no_place: return "no_place";
        default: return "unknown";
    }
}

// Function to update monthly charges based on boat length and storage type
void update_monthly_charges() {
    for (int i = 0; i < num_boats; i++) {
        Boat *boat = boats[i];
        switch (boat->place) {
            case slip:
                boat->amount_owed += boat->length * 12.50;
                break;
            case land:
                boat->amount_owed += boat->length * 14.00;
                break;
            case trailor:
                boat->amount_owed += boat->length * 25.00;
                break;
            case storage:
                boat->amount_owed += boat->length * 11.20;
                break;
            default:
                break;
        }
    }
}
// Load and save data
void load_data(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        Boat *boat = (Boat *)malloc(sizeof(Boat));
        char place_str[10];
        sscanf(line, "%[^,],%f,%[^,],%[^,],%f", boat->name, &boat->length, place_str, boat->extra.trailor_license, &boat->amount_owed);
        boat->place = StringToPlaceType(place_str);

        if (boat->place == slip) {
            boat->extra.slip_num = atoi(boat->extra.trailor_license);
        } else if (boat->place == land) {
            boat->extra.bay_letter = boat->extra.trailor_license[0];
        } else if (boat->place == storage) {
            boat->extra.storage_space = atoi(boat->extra.trailor_license);
        }
        boats[num_boats++] = boat;
    }
    fclose(file);
}

void save_data(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_boats; i++) {
        Boat *boat = boats[i];
        fprintf(file, "%s,%.0f,%s,%d,%.2f\n", boat->name, boat->length, PlaceToString(boat->place), 
                (boat->place == slip) ? boat->extra.slip_num : (boat->place == storage) ? boat->extra.storage_space : 0, 
                boat->amount_owed);
    }
    fclose(file);
}

int compare_boats(const void *a, const void *b) {
    const Boat *boat_a = *(const Boat **)a;
    const Boat *boat_b = *(const Boat **)b;
    return strcasecmp(boat_a->name, boat_b->name);
}


// Function to handle payments
void handle_payment(char *name, float amount) {
    for (int i = 0; i < num_boats; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            if (amount > boats[i]->amount_owed) {
                printf("That is more than the amount owed, $%.2f\n", boats[i]->amount_owed);
            } else {
                boats[i]->amount_owed -= amount;
                printf("Payment accepted. New balance: $%.2f\n", boats[i]->amount_owed);
            }
            return;
        }
    }
    printf("No boat with that name\n");
}

// Function to remove a boat by name
void remove_boat(char *name) {
    for (int i = 0; i < num_boats; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            free(boats[i]);
            boats[i] = boats[--num_boats];  // Replace removed boat with the last boat
            printf("Boat removed.\n");
            return;
        }
    }
    printf("No boat with that name\n");
}

// Add boat function
void add_boat(const char *csv_line) {
    if (num_boats == MAX_BOATS) {
        printf("Error: maximum number of boats reached\n");
        return;
    }

    Boat *boat = (Boat *)malloc(sizeof(Boat));
    char place_str[10];
    sscanf(csv_line, "%[^,],%f,%[^,],%[^,],%f", boat->name, &boat->length, place_str, boat->extra.trailor_license, &boat->amount_owed);
    boat->place = StringToPlaceType(place_str);

    if (boat->place == slip) {
        boat->extra.slip_num = atoi(boat->extra.trailor_license);
    } else if (boat->place == land) {
        boat->extra.bay_letter = boat->extra.trailor_license[0];
    } else if (boat->place == storage) {
        boat->extra.storage_space = atoi(boat->extra.trailor_license);
    }
    boats[num_boats++] = boat;
}

// Print the boat inventory
void print_boats() {
    printf("%-20s %-20s %-20s %-20s %-20s\n", "Name", "Length", "Place", "Extra Info", "Amount Owed");
    for (int i = 0; i < num_boats; i++) {
        Boat *boat = boats[i];
        printf("%-20s %-20.0f ", boat->name, boat->length);
        switch (boat->place) {
            case slip:
                printf("%-20s %-20d ", "Slip", boat->extra.slip_num);
                break;
            case land:
                printf("%-20s %-20c ", "Land", boat->extra.bay_letter);
                break;
            case trailor:
                printf("%-20s %-20s ", "Trailor", boat->extra.trailor_license);
                break;
            case storage:
                printf("%-20s %-20d ", "Storage", boat->extra.storage_space);
                break;
            case no_place:
                printf("%-20s %-20s ", "No Place", "N/A");
                break;
            default:
                printf("%-20s %-20s ", "Unknown", "N/A");
                break;
        }
        printf("%-20.2f\n", boat->amount_owed);
    }
}

// Free memory
void free_boats() {
    for (int i = 0; i < num_boats; i++) {
        free(boats[i]);
        boats[i] = NULL;
    }
    num_boats = 0;
}

// Menu system
void display_menu() {
    printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it: ");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    load_data(argv[1]);
    char choice;
    char input[256];
    float amount;
    printf("\nWelcome to the Boat Management System\n-------------------------------------\n");
    while (1) {
        display_menu();
        scanf(" %c", &choice);
        choice = tolower(choice);
        switch (choice) {
            case 'i':
                qsort(boats, num_boats, sizeof(Boat *), compare_boats);
                print_boats();
                break;
            case 'a':
                printf("Please enter the boat data in CSV format: ");
                scanf(" %[^\n]", input);
                add_boat(input);
                break;
            case 'r':
                printf("Please enter the boat name: ");
                scanf(" %[^\n]", input);
                remove_boat(input);
                break;
            case 'p':
                printf("Please enter the boat name: ");
                scanf(" %[^\n]", input);
                printf("Please enter the amount to be paid: ");
                scanf("%f", &amount);
                handle_payment(input, amount);
                break;
            case 'm':
                update_monthly_charges();
                printf("Monthly charges updated.\n");
                break;
            case 'x':
                save_data(argv[1]);
                free_boats();
                printf("Exiting the Boat Management System\n");
                return 0;
            default:
                printf("Invalid option %c\n", choice);
                break;
        }
    }

    return 0;
}
