#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1000

int countTotalOnes(char *str, int len) {
    int count = 0, i, j;
    for (i = 0; i < len; i++) {
        char ch = str[i];
        for (j = 7; j >= 0; j--) {
            if ((ch >> j) & 1)
                count++;
        }
    }
    return count;
}

void printBinary(char *str, int len) {
    int i, j;
    for (i = 0; i < len; i++) {
        char ch = str[i];
        for (j = 7; j >= 0; j--) {
            printf("%d", (ch >> j) & 1);
        }
        printf(" ");
    }
}

void generateParity(char *data, char *dataWithParity, int parityType) {
    int len = strlen(data);
    int ones = countTotalOnes(data, len);
    strcpy(dataWithParity, data);
    if (parityType == 1) {
        dataWithParity[len] = (ones % 2 == 0) ? '0' : '1';
    } else {
        dataWithParity[len] = (ones % 2 != 0) ? '0' : '1';
    }
    dataWithParity[len + 1] = '\0';
}

void injectError(char *frame, int dataLen, int errorChoice) {
    int totalBits = dataLen * 8;
    int bitPos1, bitPos2;
    int byteIdx, bitIdx;

    if (errorChoice == 1) {
        printf("Enter bit position to flip (1 to %d): ", totalBits);
        scanf("%d", &bitPos1);

        if (bitPos1 >= 1 && bitPos1 <= totalBits) {
            // Convert continuous bit position to string array coordinates
            byteIdx = (bitPos1 - 1) / 8;
            bitIdx = 7 - ((bitPos1 - 1) % 8);
            frame[byteIdx] ^= (1 << bitIdx);
            printf("[Channel Alert]: Injected 1-bit error.\n");
        } else {
            printf("Invalid position. No error injected.\n");
        }
    }
    else if (errorChoice == 2) {
        printf("Enter first bit position to flip (1 to %d): ", totalBits);
        scanf("%d", &bitPos1);
        printf("Enter second bit position to flip (1 to %d): ", totalBits);
        scanf("%d", &bitPos2);

        if (bitPos1 >= 1 && bitPos1 <= totalBits && bitPos2 >= 1 && bitPos2 <= totalBits) {
            // Flip first choice
            byteIdx = (bitPos1 - 1) / 8;
            bitIdx = 7 - ((bitPos1 - 1) % 8);
            frame[byteIdx] ^= (1 << bitIdx);

            // Flip second choice
            byteIdx = (bitPos2 - 1) / 8;
            bitIdx = 7 - ((bitPos2 - 1) % 8);
            frame[byteIdx] ^= (1 << bitIdx);
            printf("[Channel Alert]: Injected 2-bit error.\n");
        } else {
            printf("Invalid positions. No error injected.\n");
        }
    }
}

void checkParity(char *received, int parityType) {
    int len = strlen(received);
    char receivedData[MAX];
    strncpy(receivedData, received, len - 1);
    receivedData[len - 1] = '\0';
    printf("\n--- RECEIVER SIDE ---\n");
    printf("Received Payload (Binary): ");
    printBinary(receivedData, len - 1);

    char receivedParityBit = received[len - 1];
    printf("\nReceived Parity Bit:       %c\n", receivedParityBit);

    int dataOnes = countTotalOnes(receivedData, len - 1);
    char expectedParityBit;
    if (parityType == 1) {
        expectedParityBit = (dataOnes % 2 == 0) ? '0' : '1';
    } else {
        expectedParityBit = (dataOnes % 2 != 0) ? '0' : '1';
    }

    if (receivedParityBit == expectedParityBit) {
        printf("Status: SUCCESS (No errors detected in frame)\n");
    } else {
        printf("Status: ERROR DETECTED (Parity structural fault)\n");
    }
}

int main() {
    char data[MAX];
    char fullFrame[MAX] = {0};
    int parityType, errorChoice;
    printf("Enter DATA (Alphanumeric or Binary): ");
    if (scanf("%s", data) != 1) return 1;
    printf("Select Parity Type (1: Even, 2: Odd): ");
    if (scanf("%d", &parityType) != 1 || (parityType != 1 && parityType != 2)) {
        printf("Invalid parity choice.\n");
        return 1;
    }
    printf("Choose Error Injection Option (0: None, 1: 1-bit error, 2: 2-bit error): ");
    if (scanf("%d", &errorChoice) != 1 || errorChoice < 0 || errorChoice > 2) {
        printf("Invalid error choice.\n");
        return 1;
    }
    generateParity(data, fullFrame, parityType);
    printf("\n--- TRANSMISSION SENDER PROCESSING ---\n");
    printf("Sender Payload (Binary):   ");
    printBinary(data, strlen(data));
    printf("\nGenerated Parity Bit:      %c\n", fullFrame[strlen(data)]);

    if (errorChoice > 0) {
        injectError(fullFrame, strlen(data), errorChoice);
    } else {
        printf("\n[Channel Alert]: Clean wire transmission.\n");
    }

    checkParity(fullFrame, parityType);

    return 0;
}
