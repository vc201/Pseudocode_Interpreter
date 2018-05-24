/*
 * Date: May 30, 2012
 *
 * Description: This program is used to validate and perform commands from a text file.
 *              The moves will then be interpreted by the program and appropriate signals
 *              will be sent to the robot to contol its LED light and two motors. This program
 *              can detect errors and will return a statement if an error is found, indicating 
 *              the line the error is on. In an error is found, the program will terminate.
 *              Otherwise, the program will finish execution and terminate. Finally, this program
 *              print out the moves being done so the user can see what is happening.            
 */

/* Import modules */
#include <stdio.h>
#include <windows.h>
#include "DLPORTIO.h"
#include <string.h>
#include <time.h>

/*Function Protoytypes*/

/* Function Prototype To Output To Parallel Port */
void parallelput(unsigned char);

/*Functions that emulate robot commands*/
void forward(int);
void reverse(int);
void turnright(int);
void turnleft(int);
void pause(int);
void reset();
int loop(char[], int, FILE*);
void light(char[]);

/*Displays an error message*/
void errorMessage(int);

/*Converts strings to lowercase*/
void toLowercase(char[]);

/*Used to choose move to be performed*/
void chooseMove(int, int);

/*Functions used to validate moves, loops, and intergers, respectively*/
int isValidMove(char[], FILE*);
int isValidLoop(FILE*);
int isValidNumber(char[]);

/*Function used to trim whitespace from strings*/
char* removeWhiteSpace(char[]);

/*Used to keep track of whether light is on or off*/
int LIGHT_ON;

/*Constants*/
const int true = 1;
const int false = 0;

/*Main program*/
int main(void) 
{
    /*Variable Declarations*/
    
    /*Used to store lines from text, copies of lines, file name of text, and tokens, respectively*/
    char line[100], lineCopy[100], fileName[100], *token;
    
    /*Array of keywords that are valid commands*/
    char keywords[8][14] = {"forward", "reverse", "turnright", "turnleft", "pause", "light", "reset", "loop"};
    
    /*Variable used as a counter*/
    int counter;
    
    /*Variable used to keep track of line number in text file*/
    int lineCount = 0;
    
    /*Initializr status of the light*/
    LIGHT_ON = false;
    
    /*Variable for getting the file from the user*/
    FILE *inputFile;
    
    /*Makes sure the motors are stationary and the LED is off*/
    parallelput(0);
    
    /*Prompts user to enter a file name*/
    printf("Enter a file name: ");
    gets(fileName);
    
    /* Main loop in the program */
    
    /*If the file is valid and can be read*/
    if ( (inputFile = fopen( fileName, "r" )) != NULL )
    {
        /*While there is another line in the text file*/
        while ( !feof(inputFile) )
        {
            /*Get the line*/
            fgets( line, 99, inputFile ); 
            
            /*Convert line to lowecase*/               
            toLowercase(line);
            
            /*Creates a copy of the line for tokenization (which changes original line)*/
            strcpy(lineCopy, line);
            
            /*Increase line count by 1*/
            lineCount++; 
            
            /*Prevents iteration of previous line if very last line in text file is an empty line*/
            if( feof(inputFile) )
                break;
            
            /*If the command in the text file is valid*/
            if (isValidMove(lineCopy, inputFile) == true)
            {
                /*Display message*/
                printf("Line #%d: ", lineCount);
                /*creates a copy of the string*/
                strcpy(lineCopy, line);
                
                /*create a token using line copy*/
                token = removeWhiteSpace(strtok( lineCopy, " \t\n" ));
                
                /*Checks to see if line is not blank or a comment and displays a message indicating what was on the line*/
                if (token == NULL)
                {
                    printf("empty line\n");
                    continue;
                }
                    
                if(strncmp(token, "#", 1) == 0)
                {
                    printf("comment\n");
                    continue;
                }
                
                /*Loop used to check if move is one of the following:*/
                /*Forward, backward, turnleft, turnright, pause*/
                for (counter = 0; counter < 5; counter++)   
                {                    
                    /*If the command is one of the following*/
                    if (strcmp(token, keywords[counter]) == 0)
                    {
                        /*Select function to execture based on counter and break out of loop*/
                        chooseMove(counter, atoi(removeWhiteSpace(strtok(NULL, " \t\n")))); 
                        break;
                    }     
                }       
                
                /*Check to see if the command was to configure the light*/    
                if (strcmp(token, keywords[5]) == 0)
                    /*Execute light function*/
                    light(removeWhiteSpace(strtok( NULL, " \t\n" ))) ;
                    
                /*Check to see if the command was reset*/   
                if (strcmp(token, keywords[6]) == 0)
                    /*Execute reset function*/
                    reset();  

                /*Check to see if the command was a loop*/   
                if (strcmp(token, keywords[7]) == 0)
                    /*Adds appropriate number of lines to get current line number and execute loop function*/
                    lineCount += loop(line, atoi(removeWhiteSpace(strtok(NULL, " \t\n"))), inputFile);                             
            }
            
            /*If the move is not valid*/
            else
            {
                /*isplay error message indicating the line number in the text file where the error was found*/
                errorMessage(lineCount);
                
                /*Reset motors and LED and waits for user to press a key then exits*/
                parallelput(0);
                getch();
                return 0;   
            }
        }
        
        /* Display the results */
        
        /*If there are no more line in the file*/
        
        /*Notify user that commands are finished execuing*/
        printf("\nReached the end of the file.\nThe program will now exit.\n");
        
        /*Reset motors and LED and waits for user to press a key then exits*/
        parallelput(0);
        getch();
    }
    
    /*If file does not exist*/
    else
    {
        /*Notify user that file does not exist*/
        printf("\nThe file does not exist.\nThe program will now exit.\n");
        
        /*Wait for user to press a key then exits*/
        getch();
    }
}

/* This function outputs 1 byte (8 bits) out the parallel port */
void parallelput(unsigned char value) {
	DlPortWritePortUchar(0x378, value);
}

/*This function is used to convert strings to lowercase characters so non-case sensitive*/
/*commands may be verified*/
void toLowercase(char line[])
{
    /*Counter variable*/
    int counter;
    
    /*Converts each character in string to lowercase*/
    for (counter = 0; counter < 99; counter++)
    {
        line[counter] = tolower(line[counter]);
         
        /*Break out of loop if reached the end of the string*/        
        if (line[counter] == '\0')
            break;
    }  
}

/*This function is used to validate the commands in the text file and returns wheter a move is valid or not (true/ false)*/
int isValidMove(char line[], FILE* inputFile)
{
    /*Instance variables used in tokenization*/
    char *token, tokens[2][20];
    
    /*Array of all valid keywords (excluding comments)*/
    char keywords[10][14] = {"forward", "reverse", "turnright", "turnleft", "pause", "loop", "light", "reset", "on", "off"};
    
    /*Counter variable*/
    int counter;
    
    /*Creates token*/
    token = removeWhiteSpace(strtok( line, " \t\n" ));

    /*If token is empty or begins with a '#', the move is valid (must be a comment or empty line)*/
    if (token == NULL || strncmp(token, "#", 1) == 0)
        return true;
    
    /*Store the token*/    
    else
        strcpy(tokens[0], token);
        
    /*Create another token*/    
    token = removeWhiteSpace(strtok( NULL , " \t\n" ));
    
    /*If this token is empty and the command was reset*/
    if (token == NULL && strcmp(tokens[0], keywords[7]) == 0)
        return true;
     
    /*If this token is empty (reset is the only keyword not followed by a word or number)*/    
    else if (token == NULL)
        return false;
        
    else
        
        /*Store the second token */
        strcpy(tokens[1], token);
    
    /*Makes sure there is not a third keyword in the line (not a valid command)*/    
    if (strtok( NULL, " \t\n" ) == NULL)
    {    
        /*Checks to see if the command (first token stored is one of the commands that require a number)*/
        /*Forward, reverse, turnleft, turnrignt, pause, loop*/            
        for (counter = 0; counter  < 6; counter++)
        {   
            /*If the first token was one of the commands and the second one was a valid number*/
            if (strcmp(tokens[0], keywords[counter]) == 0 && isValidNumber(tokens[1]) == true)
            {
                /*Checks to see if the keyword was a loop (loops require special validation)*/
                if (counter == 5)
                    /*Will either be true or false*/
                    return isValidLoop(inputFile);
                    
                return true;
            }
        }   
        
        /*If the command was light and the second keyword was valid (on/off)*/
        if ((strcmp(tokens[0], keywords[6]) == 0) && (strcmp(tokens[1], keywords[8]) == 0 || strcmp(tokens[1], keywords[9]) == 0))
        {
            return true;
        }
    }
    
    /*If nothing is true then move must be invalid*/
    return false;
}

/*Chooses move based on integer passed in*/
/*The integer for choice is based on the index of items in the keywords array in the main() function*/
void chooseMove(int choice, int duration)
{
    if (choice == 0)   
        forward(duration);
    else if (choice == 1)
        reverse(duration);
    else if (choice == 2)
        turnright(duration);
    else if (choice == 3)
        turnleft(duration);
    else if (choice == 4)
        pause(duration);
}

/*Method used to turn motors so robot moves forward*/
void forward(int duration)
{
    /*Display message*/
    printf("forward %d seconds\n", duration); 
    
    /*Prevents the changing of the light status*/
    if (LIGHT_ON == true)       
        parallelput(0x0015);
       
    else
         parallelput(0x0005);
     
    /*Empty loop is used to keep track of number of seconds passed*/     
    int time1 = clock()/CLOCKS_PER_SEC;
    while (difftime(clock()/CLOCKS_PER_SEC, time1) != duration){
    }
    
    /*Turns off motors while keeping light state*/
    if (LIGHT_ON == true)       
        parallelput(0x0010);
       
    else
         parallelput(0);
}

/*Method used to turn motors so robot moves backwards*/
void reverse(int duration)
{
    /*Display message*/
    printf("reverse %d seconds\n", duration); 
    
    /*Prevents the changing of the light status*/
    if (LIGHT_ON == true)
        parallelput(0x001A);
       
    else
         parallelput(0x000A);
        
    /*Empty loop is used to keep track of number of seconds passed*/ 
    int initialTime = clock()/CLOCKS_PER_SEC;
    while (difftime(clock()/CLOCKS_PER_SEC, initialTime) != duration){
    }
    
    /*Turns off motors while keeping light state*/
    if (LIGHT_ON == true)       
        parallelput(0x0010);
       
    else
         parallelput(0);
}

/*Method used to turn motors so robot turns to the right*/
void turnright(int duration)
{
    /*Display message*/
    printf("turn right %d seconds\n", duration); 
    
    /*Prevents the changing of the light status*/
    if (LIGHT_ON == true)
        parallelput(0x0016);
       
    else
         parallelput(0x0006);
    
    /*Empty loop is used to keep track of number of seconds passed*/     
    int initialTime = clock()/CLOCKS_PER_SEC;
    while (difftime(clock()/CLOCKS_PER_SEC, initialTime) != duration){
    }
    
    /*Turns off motors while keeping light state*/
    if (LIGHT_ON == true)       
        parallelput(0x0010);
       
    else
         parallelput(0);
}

/*Method used to turn motors so robot turns to the left*/
void turnleft(int duration)
{
    /*Display message*/
    printf("turn left %d seconds\n", duration); 
    
    /*Prevents the changing of the light status*/
    if (LIGHT_ON == true)
        parallelput(0x0019);
       
    else
         parallelput(0x0009);
    
    /*Empty loop is used to keep track of number of seconds passed*/     
    int initialTime = clock()/CLOCKS_PER_SEC;
    while (difftime(clock()/CLOCKS_PER_SEC, initialTime) != duration){
    }
    
    /*Turns off motors while keeping light state*/
    if (LIGHT_ON == true)       
        parallelput(0x0010);
       
    else
         parallelput(0);
}

/*Method used to pause the program*/
void pause(int duration)
{
    /*Display message*/
    printf("pause %d seconds\n", duration);   
    _sleep(duration*1000);
}

/*Method used to reset the motors and turn off the light*/
void reset()
{
    /*Display message*/
    printf("reset\n"); 
    LIGHT_ON = false; 
    
    /*Turns motors and LED off*/ 
    parallelput(0);
}

/*Method used to turn the LED on or off*/
void light(char status[])
{   
    /*Turn on the light*/
    if (strcmp(status, "on") == 0)
    {
        /*Display message*/
        printf("light on\n");  
        LIGHT_ON = true;
        parallelput(0x0010);
    }
       
    /*Turn off the light*/     
    else if (strcmp(status, "off") == 0)
    {
        /*Display message*/
        printf("light off\n");
        LIGHT_ON = false;
        parallelput(0);
    }
}

/*This method is used to validate loops*/
int isValidLoop(FILE* inputFile)
{
    /*Used to keep track of tokens, and store lines and tokens*/
    char tokens[1][20], line[100], lineCopy[100], *token;
    
    /*Used to keep track of current position within text file*/
    fpos_t currentPosition;
    
    /*Stores current position within text file so the location can be read from again*/
    fgetpos (inputFile,&currentPosition);
    
    /*This loop is almost the same as the one in the isValidMove() function*/
    /*The only difference is that it checks for an endloop statement and then resets the position in the file after validation*/
    while ( !feof(inputFile) )
    {
        fgets( line, 99, inputFile );
        toLowercase(line);
        strcpy(lineCopy, line);
        
        token = removeWhiteSpace(strtok( lineCopy, " \t\n" ));
        
        if (strcmp(token, "\0") == 0 || strncmp(token, "#", 1) == 0)
            continue;
        
        strcpy(tokens[0], token);
        token = removeWhiteSpace(strtok( NULL, " \t\n" ));
        
        /*Check for endloop command*/
        if (strcmp(tokens[0], "endloop") == 0 && token == NULL)
        {   
            /*Go back to position in file before validation*/
            fsetpos (inputFile,&currentPosition);
            return true;
        }
        
        /*Check if moves within file are valid*/
        if (isValidMove(line, inputFile) == false)
            return false;
    }
    
    return false;    
}  

/*This method is used to verify that string are integers*/
int isValidNumber(char number[])
{
    /*Counter variable*/
    int counter;   
    
    /*Used to check each character in string*/
    for (counter = 0; counter < strlen(number); counter++)
    {
        /*If the character is not a digit*/
        if (!isdigit(number[counter]))
            return false;                 
    }
    
    /*If all characters are digits*/
    return true;
}

/*This method is used to perform the loop command*/
int loop(char line[], int iterations, FILE* inputFile)
{
    /*This method is very similar to the main() function*/
    /*The only difference is it exectues the instructions iterations number of times*/
    /*In addition, this function returns the number of lines within the loop as the last iteration*/
    /*does not alter current position within text file*/
    
    /*Same as main() function*/
    char lineCopy[100], *token;
    char keywords[7][14] = {"forward", "reverse", "turnright", "turnleft", "pause", "light", "reset"};
    int counter, counter2;
    int lineCount = 0;
    
    /*Used to keep track of current position within text file*/
    fpos_t currentPosition;
    
    /*Stores current position within text file so the location can be read from again*/
    fgetpos (inputFile,&currentPosition);
    
    strcpy(lineCopy, line);
    
    /*Display message*/
    printf("loop %d times\n", iterations);

    for (counter = 0; counter < iterations; counter++)
    { 
        /*Display message*/
        printf("\niteration #%d:\n", counter+1);
        /*Sets location within text file to line right after loop comand*/
        fsetpos (inputFile,&currentPosition);
        
        /*random string used so token does not become null after each iteration*/
        token = "reset token";
        
        /*Executes commands like in main() function as long as command is not endloop*/
        while (strcmp(token, "endloop") != 0)    
        {
             fgets( line, 99, inputFile );  
             lineCount++;
             strcpy(lineCopy, line);
             toLowercase(lineCopy); 
                
             token = removeWhiteSpace(strtok( lineCopy, " \t\n" ));
                
             if (token == NULL || strncmp(token, "#", 1) == 0)
                 continue;
            
             for (counter2 = 0; counter2 < 5; counter2++)   
             {
                 if (strcmp(token, keywords[counter2]) == 0 && counter2 < 5)
                 {
                     chooseMove(counter2, atoi(removeWhiteSpace(strtok(NULL, " \t\n")))); 
                     break;
                 }     
             }       
                
             if (strcmp(token, keywords[5]) == 0)
                 light(removeWhiteSpace(strtok( NULL, " \t\n" ))) ;

             if (strcmp(token, keywords[6]) == 0)
                 reset();  

             if (strcmp(token, keywords[7]) == 0)
                 loop(line, atoi(removeWhiteSpace(strtok(NULL, " \t\n"))), inputFile);  
         }     
     } 
     
     /*Display message*/
     printf("\nend loop\n\n");
     
     /*Returns number of line within the loop (including endloop statement)*/
     return (lineCount / iterations);  
}

/*This method is used to trim whitespace off of the end of strings*/
/*The method is used for tokens which have whitespace on them after tokenization*/
char* removeWhiteSpace(char word[])
{
    /*Don't do anything if the string is empty*/
    if (word == NULL)
        return word;
     
     /*Counter variable*/   
    int counter;
    
    /*Trim off any whitespace (tokens will only have whitespace at the end)*/
    for (counter = 0; counter < strlen(word); counter++)
        if (word[counter] == ' ' || word[counter] == '\n' )
            word[counter] = '\0';
            
    /*Returns trimmed word*/
    return word;
}

/*This method is used to display an error message to the user*/
void errorMessage(int lineNumber)
{
    /*Display error message*/
    printf("\nAn error has ocurred at line #%d in the program.\nThe program will now exit.\n", lineNumber);   
}
