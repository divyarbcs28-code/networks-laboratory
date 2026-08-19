#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SEGMENT_SIZE 8

void compute_checksum(char data[][SEGMENT_SIZE + 1], int n, char *checksum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int val = 0;
        for (int j = 0; j < 8; j++) {
            val = (val << 1) + (data[i][j] - '0');
        }
        sum += val;
    }
    while (sum > 255) {
        int carry = sum >> 8;
        sum = (sum & 255) + carry;
    }
    int check_val = ~sum & 255;
    for (int i = 7; i >= 0; i--) {
        checksum[i] = (check_val & 1) ? '1' : '0';
        check_val >>= 1;
    }
    checksum[8] = '\0';
}

int verify_checksum(char data[][SEGMENT_SIZE + 1], int n, char *checksum, int *final_sum, int *final_result) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int val = 0;
        for (int j = 0; j < 8; j++) {
            val = (val << 1) + (data[i][j] - '0');
        }
        sum += val;
    }
    int check_val = 0;
    for (int j = 0; j < 8; j++) {
        check_val = (check_val << 1) + (checksum[j] - '0');
    }
    sum += check_val;
    while (sum > 255) {
        int carry = sum >> 8;
        sum = (sum & 255) + carry;
    }
    *final_sum = sum;
    *final_result = ~sum & 255;
    return (*final_result == 0);
}

int main() {
    char raw_input[500] = "";
    char input[500] = "";
    char segments[100][SEGMENT_SIZE + 1];
    char tx_checksum[SEGMENT_SIZE + 1];
    char rx_segments[100][SEGMENT_SIZE + 1];
    char rx_checksum[SEGMENT_SIZE + 1];
    int num_segments = 0, choice, error_count, piece_num, bit_idx, input_type;
    int rx_sum = 0, rx_result = 0;

    int flipped_tracker[105][8] = {0};

    printf("Enter your data (Text or Binary): ");
    fgets(raw_input, sizeof(raw_input), stdin);
    raw_input[strcspn(raw_input, "\n")] = '\0';

    // Auto-detect input type: 1 = Binary, 2 = Text
    input_type = 1;
    for (int i = 0; raw_input[i] != '\0'; i++) {
        if (raw_input[i] != '0' && raw_input[i] != '1') {
            input_type = 2; // Found a non-binary character, treat as text
            break;
        }
    }

    if (input_type == 2) {
        printf("Detected input type: Text characters\n");
        for (int i = 0; raw_input[i] != '\0'; i++) {
            char ch = raw_input[i];
            char bin_char[9];
            for (int j = 7; j >= 0; j--) {
                bin_char[j] = (ch & 1) ? '1' : '0';
                ch >>= 1;
            }
            bin_char[8] = '\0';
            strcat(input, bin_char);
        }
    } else {
        printf("Detected input type: Binary string\n");
        strcpy(input, raw_input);
    }

    int len = strlen(input);
    int pad = (8 - (len % 8)) % 8;
    int total_len = len + pad;

    char padded[500] = "";
    for (int i = 0; i < pad; i++) {
        strcat(padded, "0");
    }
    strcat(padded, input);

    num_segments = total_len / 8;
    for (int i = 0; i < num_segments; i++) {
        for (int j = 0; j < 8; j++) {
            segments[i][j] = padded[i * 8 + j];
        }
        segments[i][8] = '\0';
    }

    compute_checksum(segments, num_segments, tx_checksum);

    printf("\nSENDER SIDE\n");
    printf("Data pieces:\n");
    for (int i = 0; i < num_segments; i++) {
        printf("Piece %d: %s\n", i + 1, segments[i]);
    }
    printf("Checksum code: %s\n", tx_checksum);

    for (int i = 0; i < num_segments; i++) {
        strcpy(rx_segments[i], segments[i]);
    }
    strcpy(rx_checksum, tx_checksum);

    printf("\n--- CHOOSE AN OPTION ---\n");
    printf("1. No error\n");
    printf("2. 1 Bit Error\n");
    printf("3. Add many wrong bits\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 2) {
        printf("Enter Piece number to change (Enter %d for Checksum code): ", num_segments + 1);
        scanf("%d", &piece_num);
        printf("Enter bit position to change (1 to 8): ");
        scanf("%d", &bit_idx);

        bit_idx--;
        if (piece_num <= num_segments) {
            rx_segments[piece_num - 1][bit_idx] = (rx_segments[piece_num - 1][bit_idx] == '0') ? '1' : '0';
        } else {
            rx_checksum[bit_idx] = (rx_checksum[bit_idx] == '0') ? '1' : '0';
        }
    } else if (choice == 3) {
        printf("How many wrong bits do you want to add? ");
        scanf("%d", &error_count);
        for (int e = 0; e < error_count; e++) {
            int valid_input = 0;
            while (valid_input == 0) {
                printf("\nFor error bit %d:\n", e + 1);
                printf("Enter Piece number to change (Enter %d for Checksum code): ", num_segments + 1);
                scanf("%d", &piece_num);
                printf("Enter bit position to change (1 to 8): ");
                scanf("%d", &bit_idx);

                int internal_bit_idx = bit_idx - 1;

                if (flipped_tracker[piece_num][internal_bit_idx] == 1) {
                    printf("Error: This exact bit position has already been flipped! Choose a different bit.\n");
                } else {
                    flipped_tracker[piece_num][internal_bit_idx] = 1;
                    valid_input = 1;

                    if (piece_num <= num_segments) {
                        rx_segments[piece_num - 1][internal_bit_idx] = (rx_segments[piece_num - 1][internal_bit_idx] == '0') ? '1' : '0';
                    } else {
                        rx_checksum[internal_bit_idx] = (rx_checksum[internal_bit_idx] == '0') ? '1' : '0';
                    }
                }
            }
        }
    } else {
        printf("\nData sent without changes\n");
    }

    printf("\nRECEIVER SIDE\n");
    printf("Received pieces:\n");
    for (int i = 0; i < num_segments; i++) {
        printf("Piece %d: %s\n", i + 1, rx_segments[i]);
    }
    printf("Received Checksum code: %s\n", rx_checksum);

    int is_valid = verify_checksum(rx_segments, num_segments, rx_checksum, &rx_sum, &rx_result);

    char rx_sum_bin[9];
    char rx_result_bin[9];
    int temp_sum = rx_sum;
    int temp_res = rx_result;
    for (int j = 7; j >= 0; j--) {
        rx_sum_bin[j] = (temp_sum & 1) ? '1' : '0';
        temp_sum >>= 1;
        rx_result_bin[j] = (temp_res & 1) ? '1' : '0';
        temp_res >>= 1;
    }
    rx_sum_bin[8] = '\0';
    rx_result_bin[8] = '\0';

    printf("\nReceiver calculations:\n");
    printf("Sum value (1s complement format): %s (Decimal: %d)\n", rx_sum_bin, rx_sum);
    printf("Final check value (Inverted sum): %s (Decimal: %d)\n", rx_result_bin, rx_result);

    if (is_valid) {
        printf("\nNo errors found.Data Transimmited successfully.\n");

        if (input_type == 2) {
            printf("Decoded Text: ");
            for (int i = 0; i < num_segments; i++) {
                char ch = 0;
                for (int j = 0; j < 8; j++) {
                    ch = (ch << 1) + (rx_segments[i][j] - '0');
                }
                if(ch >= 32 && ch <= 126) {
                    printf("%c", ch);
                }
            }
            printf("\n");
        }
    } else {
        printf("\nResult:Errors found.Data Discarded\n");
    }

    return 0;
}
