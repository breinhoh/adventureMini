#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int check_done(int x[7][8]);
int checkChosen(int x[7], int y);

int main()
{
    //set up random number generator
    srand(time(NULL));
    
    //set up possible room names
    char names[10][10] = {"Amir", "Suri", "Dave", "Tess", "Dane", "Sara", "Evan", "Ruth", "Thor", "Anne"};
    
    
    /**************************************************
     This section creates the randomized map for each game.
     **************************************************/
    
    
    //select room names
    int chosen_nums[7] = {11,11,11,11,11,11,11};
    int sevNames;
    for (sevNames = 0; sevNames < 7; sevNames++)
    {
        while (1)
        {
            int name_select = rand() % 10;
            if (checkChosen(chosen_nums, name_select))
            {
                chosen_nums[sevNames] = name_select;
                break;
            }
        }
        
    }
    
    //create room matrix
    int rooms[7][8] = {
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
    };
    //make rooms unconnectable to themselves
    int index;
    for (index = 0; index < 7; index++)
    {
        rooms[index][index] = 8;
    }
    
    //create random room connections
    //draw a number for a row that is not full
    do {
        int rowAdd = rand() % 7;
        //check that row isn't already full
        if (rooms[rowAdd][7] != 6)
        {
            //draw a column that isnt already chosen
            while (1)
            {
                //draw column number
                int colAdd = rand() % 7;
                //check that it is not the same as the row or already chosen
                if (colAdd != rowAdd && rooms[rowAdd][colAdd] != 9)
                {
                    //mark connections with 9 and incrament counters
                    rooms[rowAdd][colAdd] = 9;
                    rooms[colAdd][rowAdd] = 9;
                    rooms[rowAdd][7]++;
                    rooms[colAdd][7]++;
                    break;
                }
            }
        }
    } while(!check_done(rooms));
    
    //choose start and end
    int startRoom = rand() % 7;
    int endRoom = 8;
    do {
        endRoom = rand() % 7;
    } while (endRoom == startRoom && endRoom < 7);
    
    
    
    /**************************************************
     This section write the created map to the files they
     will be read from. It also creates the directory they
     are in and a temporary file for storing the path history.
     **************************************************/
    
    
    //get process id
    int pid = getpid();
    //create directory pointer
    char *dirName = malloc(20);
    //create dirname from parts
    snprintf(dirName, 20, "breinhoh.rooms.%d", pid);
    
    struct stat st = {0};
    
    //create directory
    if (stat(dirName, &st) == -1) {
        mkdir(dirName, 0755);
    }
    
    //open file in directory
    int fileCount;
    for (fileCount = 0; fileCount < 7; fileCount++)
    {
        //create file
        char filepath[30];
        strcpy(filepath, dirName);
        strcat(filepath, "/");
        strcat(filepath, names[chosen_nums[fileCount]]);
        FILE* f = fopen(filepath, "w");
        if (f < 0) {
            perror("CLIENT:\n");
            exit(1);
        }
        //write data to file
        //write name to file
        fprintf(f, "ROOM NAME: %s\n", names[chosen_nums[fileCount]]);
        //write connections to file
        int conCount2 = 1;
        int conCount;
        for (conCount = 0; conCount < 7; conCount++)
        {
            if (rooms[fileCount][conCount] == 9)
            {
                fprintf(f, "CONNECTION %d: %s\n", conCount2, names[chosen_nums[conCount]]);
                conCount2++;
            }
        }
        //write start, end, or mid
        if (fileCount == startRoom)
        {
            fprintf(f, "ROOM TYPE: START_ROOM");
        }
        else if (fileCount == endRoom)
        {
            fprintf(f, "ROOM TYPE: END_ROOM");
        }
        else
        {
            fprintf(f, "ROOM TYPE: MID_ROOM");
        }
        //close file
        fclose(f);
    }
    //create history file
    char hfilepath[30];
    strcpy(hfilepath, dirName);
    strcat(hfilepath, "/history");
    FILE* hf = fopen(hfilepath, "w+");
    if (hf < 0) {
        perror("CLIENT:\n");
        exit(1);
    }
    
    
    /**************************************************
     This section is the actual game.  It locates the start
     file and reads it.  Then it outputs the right info
     to the screen. Then it takes user input and if it
     checks out it records the right data finds the new
     file and starts over.  If the user data does not it
     reprints the current files data and ask again.
     **************************************************/
    
    
    //get start file name
    char currRoom[5];
    strcpy(currRoom, names[chosen_nums[startRoom]]);
    int stepCount = 0;
    int destNotReached = 1;
    //This loop is the game itself each room is read here
    while (destNotReached)
    {
        int roomNum = 0;
        //open file
        char roomfile[30];
        strcpy(roomfile, dirName);
        strcat(roomfile, "/");
        strcat(roomfile, currRoom);
        int file_descriptor;
        ssize_t nread;
        char buffer[512];
        
        file_descriptor = open(roomfile, O_RDONLY);
        if (file_descriptor == -1)
        {
            exit(1);
        }
        //save name and connections to array
        char data[8][10] = {"", "", "", "", "", "", "", ""};
        nread = read(file_descriptor, buffer, 512);
        char fileText[512];
        strcpy(fileText, buffer);
        memset(buffer, 0, 512);
        char * token = strtok(fileText, ":");
        token = strtok(NULL, " \n");
        strcpy(data[roomNum], token);
        roomNum++;
        token = strtok(NULL, " \n");
        while (token[0] == 'C')
        {
            token = strtok(NULL, " \n");
            token = strtok(NULL, " \n");
            strcpy(data[roomNum], token);
            roomNum++;
            token = strtok(NULL, "\r R");
        }
        token = strtok(NULL, ":");
        token = strtok(NULL, " \n");
        strcpy(data[7], token);
        close(file_descriptor);
        
        int verified = 1;
        //this loop is to verify the user input
        while (verified)
        {
            //check if file is end file
            if (!strcmp(data[7], "END_ROOM"))
            {
                destNotReached = 0;
                break;
            }
            //print info
            printf("CURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS: ", data[0]);
            int conns;
            for (conns = 1; conns < 7; conns++)
            {
                if (strlen(data[conns]) == 0)
                {
                    printf(".\nWHERE TO? >");
                    break;
                }
                else
                {
                    printf("%s", data[conns]);
                    if (conns != 6)
                    {
                        if (strlen(data[conns + 1]) != 0)
                        {
                            printf(", ");
                        }
                    }
                    else
                    {
                        printf(".\nWHERE TO? >");
                    }
                }
            }
            //take user input
            char go2room[100];
            fgets(go2room, 100, stdin);
            go2room[strlen(go2room) - 1] = '\0';
            printf("\n");
            //check if user input is valid
            int roomCheck;
            for (roomCheck = 1; roomCheck < 7; roomCheck++)
            {
                if (!strcmp(go2room, data[roomCheck]))
                {
                    verified = 0;
                    strcpy(currRoom, go2room);
                    stepCount++;
                    //read new room to hostory file
                    fprintf(hf, go2room);
                    fprintf(hf, "\n");
                    
                }
            }
            //find new file or print an error response
            if (!verified)
            {
                break;
            }
            else
            {
                printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
            }
        }
    }
    //the game is over print the game data and delete the temp file.
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);
    rewind(hf);
    char let[512];
    while(fscanf(hf,"%[^\n]\n",let)!=EOF)
    {
        
        printf("%s\n", let);
    }
    // file close
    fclose(hf);
    
    remove(hfilepath);
    
    return 0;
}

/**************************************************
 This function is used when creating the map.  It
 checks if the map is complete.
 **************************************************/
int check_done(int x[7][8])
{
    int index;
    for (index = 0; index < 7; index++)
    {
        if (x[index][7] < 3)
        {
            return 0;
        }
    }
    return 1;
}

/**************************************************
 This function is used when creating the map.  It
 checks if a particular connection has already been made.
 **************************************************/
int checkChosen(int x[7], int y)
{
    int index;
    for (index = 0; index < 7; index++)
    {
        if (x[index] == y)
        {
            return 0;
        }
    }
    return 1;
}