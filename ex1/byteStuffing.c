#include <stdio.h>
#include <string.h>
#define FLAG 126
#define ESC 163
void printAsBinary(int value)
{
    int i;
    for(i=7;i>=0;i--)
        printf("%d",(value>>i)&1);
    printf(" ");
}
int main()
{
    char text[100];
    int data[100];
    int frame[200];
    int result[100];
    int i;
    char errChoice;
    int errType;
    int bytePos1, bytePos2;
    printf("Enter text ('F' for FLAG and 'E' for ESC): ");
    scanf("%s",text);
    int dataLen=strlen(text);
    for(i=0;i<dataLen;i++)
    {
        if(text[i]=='F')
            data[i]=FLAG;
        else if(text[i]=='E')
            data[i]=ESC;
        else
            data[i]=text[i];
    }
    printf("\nSENDER SIDE\n");
    printf("Original Text: %s\n\n",text);
    int fIdx=0;
    frame[fIdx++]=FLAG;
    for(i=0;i<dataLen;i++)
    {
        if(data[i]==FLAG)
        {
            frame[fIdx++]=ESC;
            frame[fIdx++]=FLAG;
        }
        else if(data[i]==ESC)
        {
            frame[fIdx++]=ESC;
            frame[fIdx++]=ESC;
        }
        else
            frame[fIdx++]=data[i];
    }
    frame[fIdx++]=FLAG;
    printf("\nFinal Frame\n");
    printf("Framed Characters   : ");
    for(i=0;i<fIdx;i++)
    {
        if(frame[i]==FLAG)
            printf("F\t");
        else if(frame[i]==ESC)
            printf("E\t");
        else
            printf("%c\t",frame[i]);
    }
    printf("\nFramed Packet ASCII : ");
    for(i=0;i<fIdx;i++)
        printf("%-8d ",frame[i]);
    printf("\nFramed Packet Binary: ");
    for(i=0;i<fIdx;i++)
        printAsBinary(frame[i]);
    printf("\n");
    printf("\nDo you want to induce an error? (Y/N): ");
    scanf(" %c",&errChoice);
    if(errChoice=='Y' || errChoice=='y')
    {
        printf("\n1. 1 Byte Error");
        printf("\n2. 2 Bytes Error");
        printf("\nEnter your choice: ");
        scanf("%d",&errType);
        if(errType==1)
        {
            printf("Enter Byte Position (1-%d): ",fIdx);
            scanf("%d",&bytePos1);
            if(bytePos1>=1 && bytePos1<=fIdx)
            {
                frame[bytePos1-1]^=0xFF;
                printf("\n1 Byte Error Induced Successfully.\n");
            }
            else
            {
                printf("\nInvalid Position.\n");
                return 0;
            }
        }
        else if(errType==2)
        {
            printf("Enter 1st Byte Position (1-%d): ",fIdx);
            scanf("%d",&bytePos1);
            printf("Enter 2nd Byte Position (1-%d): ",fIdx);
            scanf("%d",&bytePos2);
            if(bytePos1>=1 && bytePos1<=fIdx && bytePos2>=1 && bytePos2<=fIdx)
            {
                frame[bytePos1-1]^=0xFF;
                frame[bytePos2-1]^=0xFF;
                printf("\n2 Bytes Error Induced Successfully.\n");
            }
            else
            {
                printf("\nInvalid Position.\n");
                return 0;
            }
        }
        else
        {
            printf("\nInvalid Choice.\n");
            return 0;
        }
        printf("\nFRAME AFTER ERROR ===\n");
        printf("Framed Characters: ");
        for(i=0;i<fIdx;i++)
        {
            if(frame[i]==FLAG)
                printf("F\t");
            else if(frame[i]==ESC)
                printf("E\t");
            else if(frame[i]>=32 && frame[i]<=126)
                printf("%c\t",frame[i]);
            else
                printf("?\t");
        }
        printf("\nFramed Packet ASCII : ");
        for(i=0;i<fIdx;i++)
            printf("%-8d ",frame[i]);
        printf("\nFramed Packet Binary: ");
        for(i=0;i<fIdx;i++)
            printAsBinary(frame[i]);
        printf("\n");
        printf("\nError Detected in receiver side\n");
        return 0;
    }
    printf("\nRECEIVER SIDE\n");
    int rIdx=0;
    for(i=1;i<fIdx-1;i++)
    {
        if(frame[i]==ESC)
        {
            i++;
            result[rIdx++]=frame[i];
        }
        else
            result[rIdx++]=frame[i];
    }
    printf("\nFINAL RESULT\n");
    printf("Correct Destuffed Ans: ");
    for(i=0;i<rIdx;i++)
    {
        if(result[i]==FLAG)
            printf("F\t");
        else if(result[i]==ESC)
            printf("E\t");
        else
            printf("%c",result[i]);
    }
    printf("\n");
    printf("Final Clean ASCII: ");
    for(i=0;i<rIdx;i++)
        printf("%d ",result[i]);
    printf("\nFinal Clean Binary ");
    for(i=0;i<rIdx;i++)
        printAsBinary(result[i]);
    printf("\n");
    return 0;
}
