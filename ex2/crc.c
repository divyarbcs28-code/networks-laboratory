#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BITS 2048

int is_binary_string(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != '0' && str[i] != '1') return 0;
    }
    return 1;
}

void data_to_binary(const char *input, char *output) {
    if (is_binary_string(input) && strlen(input) > 0) {
        strcpy(output, input);
        return;
    }
    output[0] = '\0';
    char byte_str[9];
    for (int i = 0; input[i] != '\0'; i++) {
        unsigned char ch = input[i];
        for (int b = 7; b >= 0; b--) {
            byte_str[7 - b] = (ch & (1 << b)) ? '1' : '0';
        }
        byte_str[8] = '\0';
        strcat(output, byte_str);
    }
}

// Updated validation function meeting only your exact criteria
int validate_generator(const char *gen, char *error_msg) {
    int len = strlen(gen);

    // Check if it only contains 0 and 1
    for (int i = 0; i < len; i++) {
        if (gen[i] != '0' && gen[i] != '1') {
            strcpy(error_msg, "Generator must contain only '0' and '1'.");
            return 0;
        }
    }

    // Requirement 1: At least 2 bits long
    if (len < 2) {
        strcpy(error_msg, "Generator must have at least 2 bits.");
        return 0;
    }

    // Requirement 2: Must start and end with '1'
    if (gen[0] != '1' || gen[len - 1] != '1') {
        strcpy(error_msg, "Generator must start and end with '1'.");
        return 0;
    }

    return 1; // Valid polynomial
}

void crc_division(const char *dividend, const char *generator, char *remainder) {
    int gen_len = strlen(generator);
    int div_len = strlen(dividend);
    char temp[MAX_BITS];
    strcpy(temp, dividend);

    for (int i = 0; i <= div_len - gen_len; i++) {
        if (temp[i] == '1') {
            for (int j = 0; j < gen_len; j++) {
                temp[i + j] = (temp[i + j] == generator[j]) ? '0' : '1';
            }
        }
    }
    strncpy(remainder, &temp[div_len - (gen_len - 1)], gen_len - 1);
    remainder[gen_len - 1] = '\0';
}

void run_receiver_check(const char *received_codeword, const char *generator) {
    int gen_len = strlen(generator);
    char receiver_remainder[MAX_BITS];

    crc_division(received_codeword, generator, receiver_remainder);

    printf("Received Codeword:  %s\n", received_codeword);
    printf("Receiver Remainder: %s\n", receiver_remainder);

    int error_detected = 0;
    for (int i = 0; i < gen_len - 1; i++) {
        if (receiver_remainder[i] == '1') {
            error_detected = 1;
            break;
        }
    }

    if (error_detected) {
        printf("Status: ERROR DETECTED\n");
    } else {
        printf("Status: SUCCESS (No Errors)\n");
    }
}

int main() {
    char input_data[MAX_BITS];
    char generator[MAX_BITS];
    char error_msg[256];

    char binary_data[MAX_BITS];
    char padded_data[MAX_BITS];
    char remainder[MAX_BITS];
    char transmitted_codeword[MAX_BITS];
    char simulated_packet[MAX_BITS];

    printf("Enter Sender Data: ");
    if (fgets(input_data, sizeof(input_data), stdin) == NULL) return 1;
    input_data[strcspn(input_data, "\n")] = '\0';

    printf("Enter Generator Polynomial: ");
    if (fgets(generator, sizeof(generator), stdin) == NULL) return 1;
    generator[strcspn(generator, "\n")] = '\0';

    if (!validate_generator(generator, error_msg)) {
        printf("Error: %s\n", error_msg);
        return 1;
    }

    data_to_binary(input_data, binary_data);
    int gen_len = strlen(generator);
    strcpy(padded_data, binary_data);
    for (int i = 0; i < gen_len - 1; i++) {
        strcat(padded_data, "0");
    }

    crc_division(padded_data, generator, remainder);

    strcpy(transmitted_codeword, binary_data);
    strcat(transmitted_codeword, remainder);

    printf("\n--- SENDER SIDE ---\n");
    printf("Binary Data:        %s\n", binary_data);
    printf("Calculated CRC:     %s\n", remainder);
    printf("Transmitted Frame:  %s\n", transmitted_codeword);

    int choice;
    printf("\nSelect Error Option (0: No Error, 1: 1-Bit Error, 2: Multi-Bit Error): ");
    if (scanf("%d", &choice) != 1) return 1;

    int code_len = strlen(transmitted_codeword);
    strcpy(simulated_packet, transmitted_codeword);

    if (choice == 0) {
        printf("\n--- RECEIVER SIDE ---\n");
        run_receiver_check(transmitted_codeword, generator);
    }
    else if (choice == 1) {
        printf("Enter bit index to flip (0 to %d): ", code_len - 1);
        int single_idx;
        if (scanf("%d", &single_idx) == 1 && single_idx >= 0 && single_idx < code_len) {
            simulated_packet[single_idx] = (simulated_packet[single_idx] == '0') ? '1' : '0';
            printf("\n--- RECEIVER SIDE ---\n");
            run_receiver_check(simulated_packet, generator);
        } else {
            printf("Invalid index.\n");
        }
    }
    else if (choice == 2) {
        printf("How many bits do you want to flip? ");
        int num_errors;
        if (scanf("%d", &num_errors) == 1 && num_errors > 0) {
            for (int i = 0; i < num_errors; i++) {
                printf("Enter bit index %d to flip (0 to %d): ", i + 1, code_len - 1);
                int mult_idx;
                if (scanf("%d", &mult_idx) == 1 && mult_idx >= 0 && mult_idx < code_len) {
                    simulated_packet[mult_idx] = (simulated_packet[mult_idx] == '0') ? '1' : '0';
                }
            }
            printf("\n--- RECEIVER SIDE ---\n");
            run_receiver_check(simulated_packet, generator);
        } else {
            printf("Invalid input.\n");
        }
    }
    else {
        printf("Invalid choice.\n");
    }

    return 0;
}
