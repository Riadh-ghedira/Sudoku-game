#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define taille 9
int score = 0;
int error = 0;
int grid[taille][taille];
int solution[taille][taille];

void game(int slot);
void submit();

/*partie login et gestion compte*/
struct account
{
    char name[20] ;
    char password[15];
    int score ;
    int tgp ; // total games played
    int tgw ; // total games won
    char nfg[10][taille * taille] ;//non finished games
};

struct account ca;

void save() {
    FILE *file;
    char filename[25];
    strcpy(filename, ca.name);
    strcat(filename, ".txt");
    file = fopen(filename, "w");
    if (file) 
    {
        fprintf(file, "userName:%s\npassword:%s\nScore:%d\nTGP:%d\nTGW:%d\nNFG:%s\n", ca.name, ca.password, ca.score, ca.tgp, ca.tgw, ca.nfg);
        fclose(file);
    } 
    else 
    {
        printf("Error: Unable to save account data.\n");
    }
}
void saveToDB(){
    FILE *accountDB = fopen("db.txt", "a");
    if (accountDB) 
    {
        fprintf(accountDB, "%s.txt/%s\n", ca.name, ca.password);
        fclose(accountDB);
    } 
    else 
    {
        fclose(accountDB);
        accountDB = fopen("db.txt","w");
        fprintf(accountDB, "%s.txt/%s\n", ca.name,ca.password);
        fclose(accountDB);
    }
}
void createAccount(){
    printf("Enter username: ");
    scanf("%s", ca.name);
    printf("Enter password: ");
    scanf("%s", ca.password);
    ca.score = 0;
    ca.tgp = 0;
    ca.tgw = 0;
    memset(ca.nfg, 0, sizeof(ca.nfg)); 
    save();
    saveToDB();
}
int getAccounts(char *name , char *pswd) {
    FILE *accountDB = fopen("db.txt", "r");
    char line[50], dbName[20], dbPassword[15];

    if (!accountDB) return 2;

    while (fgets(line, sizeof(line), accountDB)) {
        sscanf(line, "%[^.].txt/%s", dbName, dbPassword);
        if (strcmp(name, dbName) == 0) {
            fclose(accountDB);
            strcpy(pswd, dbPassword);
            return 0; 
        }
    }
    fclose(accountDB);
    return 2; 
}
int verifAccount(char *name, const char *pswd) {
    char storedPassword[15];
    int result = getAccounts(name, storedPassword);
    if (result == 2) return 2; 
    return strcmp(pswd, storedPassword) == 0 ? 0 : 1; 
}
void syncAccount(char *name) {
    FILE *accountFile;
    char filename[25];
    strcpy(filename, name);
    strcat(filename, ".txt");
    accountFile = fopen(filename, "r");
    if (accountFile == NULL) {
        printf("Error loading account data.\n");
        return;
    }
    fscanf(accountFile, "userName:%s\npassword:%s\nScore:%d\nTGP:%d\nTGW:%d\nNFG:%s\n", ca.name, ca.password, &ca.score, &ca.tgp, &ca.tgw,&ca.nfg);
    fclose(accountFile);
}
int login(){

    char name[20];
    char pswd[15];
    char command[20];

    while (true) {
        printf("Enter username: ");
        scanf("%s", name);
        printf("Enter password: ");
        scanf("%s", pswd);

        int res = verifAccount(name, pswd);
        
        if (res == 0) 
        {
            syncAccount(name);
            printf("Login successful.\n");
            return 0;
        } 
        else if (res == 1) 
        {
            printf("Wrong password, try again.\n");
        } 
        else 
        {
            printf("No account found. Type 'sign-up' to create a new account or 'cancel' to play as a guest: ");
            scanf("%s", command);
            if (strcmp(command, "sign-up") == 0) 
            {
                createAccount();
                return 0;
            }
            else if (strcmp(command, "cancel") == 0) 
            {
                strcpy(ca.name, "guest");
                return 0;
            }
        }
    }
}

/*game DB*/
void stringToGrid(const char *slot, int grid[taille][taille]) {
    int k = 0;
    for (int i = 0; i < taille; i++) {
        for (int j = 0; j < taille; j++) {
            grid[i][j] = slot[k++] - '0'; 
        }
    }
}
void gridToString(int grid[taille][taille], char *slot) {
    int k = 0;
    for (int i = 0; i < taille; i++) {
        for (int j = 0; j < taille; j++) {
            slot[k++] = grid[i][j] + '0'; 
        }
    }
    slot[k] = '\0';

}
void loadGame(){ 
    int slot ;
    if (strcmp(ca.name, "guest") == 0) {
        printf("Guest users cannot load saved games.\n");
        return;
    }
    printf("Available saved games:\n");
    for (int i = 0; i < 10; i++) {
        if (strlen(ca.nfg[i]) > 0) {
            printf("Slot %d: Saved game\n", i + 1);
        }
    }

    printf("Enter the slot number to load (1-10): ");
    
    scanf("%d", &slot);
    if ((slot < 1) || (slot > 10) || (ca.nfg[slot - 1] == NULL || strlen(ca.nfg[slot - 1]) == 0)) {
        printf("Invalid slot selected or no game saved in this slot.\n");
        return;
    }

    stringToGrid(ca.nfg[slot - 1], grid);
    printf("Game loaded from slot %d.\n", slot);
    game(slot);
    
}
void deleteGame() {
    printf("Available saved games:\n");
    for (int i = 0; i < 10; i++) {
        if (strlen(ca.nfg[i]) > 0) {
            printf("Slot %d: Saved game\n", i + 1);
        }
    }

    printf("Enter the slot number to delete (1-10): ");
    int slot;
    scanf("%d", &slot);

    if (slot < 1 || slot > 10 || strlen(ca.nfg[slot - 1]) == 0) {
        printf("Invalid slot selected or no game saved in this slot.\n");
        return;
    }

    memset(ca.nfg[slot - 1], 0, sizeof(ca.nfg[slot - 1]));
    printf("Saved game in slot %d deleted.\n", slot);
}

/*verification*/
bool isValid(int row, int col, int num) {
    for (int x = 0; x < taille; x++) {
        if (grid[row][x] == num || grid[x][col] == num || 
            grid[row - row % 3 + x / 3][col - col % 3 + x % 3] == num) {
            return false;
        }
    }
    return true;
}

/*generate grid*/
bool fillGrid(int grid[taille][taille], int row, int col) {
    if (row == taille - 1 && col == taille) return true;
        
    if (col == taille) 
    {
        row++;
        col = 0;
    }

    if (grid[row][col] != 0) return fillGrid(grid, row, col + 1);
        
    int nums[taille];

    for (int i = 0; i < taille; i++) 
    {
        nums[i] = i + 1;
    }

    for (int i = 0; i < taille; i++) 
    {
        int j = rand() % taille;
        int temp = nums[i];
        nums[i] = nums[j];
        nums[j] = temp;
    }

    for (int i = 0; i < taille; i++) 
    {
        if (isValid(grid, row, col, nums[i])) 
        {
            grid[row][col] = nums[i];
            if (fillGrid(grid, row, col + 1)) return true;   
            grid[row][col] = 0;
        }
    }

    return false;
}
void generate(){
    srand(time(0));
    fillGrid(0, 0);
    for (int i = 0; i < taille; i++) {
        for (int j = 0; j < taille; j++) {
            solution[i][j] = grid[i][j];
        }
    }  
}

/*info*/
void header(){
    printf("User Name: %s", ca.name);
    if (strcmp(ca.name , "guest")==0)
    {
        printf("\nType 'Login' to enter your existing account or 'Sign-up' to create new\n");

    }else
    {
        printf("          Score: %d        error: %d/3 \n" , ca.score , error);
        printf("Type 'info' to see your account info\n");
    }
}
void welcom(){
    printf("\n                        Welcome to Sudoku\n\n");
    header();
    printf("To reread the instructions, type 'instruction'.\n");
    
}
void rules(){
    printf("\n");
    printf("Rules:\n");
    printf("Every number from 1 to 9 must appear once in each row, column, and 3x3 box.\n\n");
    printf("How to play:\n");
    printf("-) Zeroes in the grid represent empty cells.\n");
    printf("-) Type 'row col num' to fill a value in the grid.\n");
    printf("-) Type 'save' to save unfineshed game.\n");
    printf("-) Type 'load-game' to load old saved game.\n");
    printf("-) Type 'deleat-game' to deleat old saved game.\n");
    printf("-) Type 'new game' to start a new game.\n");
    printf("-) Type 'exit' to quit.\n");
} 
void info(){
    
}

/*dificulty setting and initiation*/
void setDifficulty(int n) {
    int hiden = 0;
    while (hiden < n) {
        int row = rand() % taille;
        int col = rand() % taille;
        if (grid[row][col] != 0) {
            grid[row][col] = 0;
            hiden++;
        }
    }
}
void setgrid(char level[]){
    int n;
    if (strcmp(level, "hard") == 0) {//strcmp fonction predifini mil bib string.h tcompari les string lettre par lettre
        n = 55;
        score = 100 ;
    } else if (strcmp(level, "medium") == 0) {
        n = 35;
        score = 60 ;
    } else {
        n = 2;
        score = 20 ;
    }
    generate();
    setDifficulty(n);
}

/*el affichage*/
void printGrid() {
    printf("\n");
    printf("     #-------#-------#-------#\n");
    for (int i = 0; i < taille; i++) {
        
        if ((i == 3)||(i == 6))
        {
            printf("     #-------+-------+-------#\n");
        }
        printf("     | ");
        for (int j = 0; j < taille; j++) {
            printf("%d ", grid[i][j]);
            if ((j==2)||(j==5))
            {
                printf("| ");
            }
        }
        printf("|\n");
    }
    printf("     #-------#-------#-------#");
    printf("\n");
}
void count() {
    int sub = 0;
    int counts[taille] = {0};

    for (int i = 0; i < taille; i++) {
        for (int j = 0; j < taille; j++) {
            if (grid[i][j] > 0 && grid[i][j] <= taille) {
                counts[grid[i][j] - 1]++;
            }
        }
    }

    for (int i = 0; i < taille; i++) {
        printf("%d:%d ", i + 1, (9 - counts[i]));
        sub += counts[i];
    }
    printf("\n\n");
    if (sub == 81)
    {
        submit();
    }
    
}

/*function*/
void saving(){
    char response[10];
    if (strcmp(ca.name , "guest") == 0)
    {
        printf("You can't save. Need to have an account.\n");
        printf("Type 'Login' to enter your existing account or 'Sign-up' to create new\n");
        scanf("%s", response);
        if (strcmp(response, "login")==0)
        {
            login();
            saving();
        }
        else if (strcmp(response, "sign-up")==0)
        {
            createAccount();
            saving();
        }

        }
        else
        {
            save();
            printf("Exiting the game...\n");
            printf("thank you for playing soduko. See you next time.\n");
        }
}
void gameExit(){
    printf("Do you want to save progress? Yes/No ?\n");
    char response[10];
    scanf("%s", response);
    if (strcmp(response, "yes") == 0) {
        saving(); 
    }
    else
    {
        printf("Exiting the game...\n");
        printf("thank you for playing soduko. See you next time.\n");
    }
}
void submit(){
    bool correct = true;
    for (int i = 0; i < taille; i++) {
        for (int j = 0; j < taille; j++) {
            if (grid[i][j] != solution[i][j]) {
                correct = false;
                break;
            }
        }
    }
    if (correct) {
        char command[20];
        printf("Congratulations! You've completed the puzzle!\n");
        ca.tgw=ca.tgw+1;
        ca.score+=score;
        save();
        printf("Do you want to play again? Type 'Start' to play or 'Exit' to quit the game.");
        scanf("%s", command);
        if (strcmp(command, "start")==0)
        {
            game(-1);
        }
        else if (strcmp(command, "exit") == 0) {
            gameExit();
            return;
        }
    } 
    else {
        printf("Some entries are incorrect. Please try again.\n");
    }
}
void saveGame(){
    if (strcmp(ca.name, "guest") == 0) {
        printf("Guest users cannot save games.\n");
        return;
    }

    char gidData[taille * taille + 1];
    gridToString(grid, gidData);

    for (int i = 0; i < 10; i++) {
        if (strlen(ca.nfg[i]) == 0) { 
            strcpy(ca.nfg[i], gidData);
            printf("Game saved in slot %d.\n", i + 1);
            save();
            return;
        }
    }

    printf("No available slots to save the game. Please delete an old game.\n");
}
void delGame(){
        printf("Available saved games:\n");
    for (int i = 0; i < 10; i++) {
        if (strlen(ca.nfg[i]) > 0) {
            printf("Slot %d: Saved game\n", i + 1);
        }
    }

    printf("Enter the slot number to delete (1-10): ");
    int slot;
    scanf("%d", &slot);

    if (slot < 1 || slot > 10 || strlen(ca.nfg[slot - 1]) == 0) {
        printf("Invalid slot selected or no game saved in this slot.\n");
        return;
    }

    memset(ca.nfg[slot - 1], 0, sizeof(ca.nfg[slot - 1]));
    printf("Saved game in slot %d deleted.\n", slot);
}

/*game*/
void game(int slot){
    int row, col, num;
    char command[20];
    static int M = 1;;

    if (slot == -1)
    {
        printf("Choose difficulty: easy, medium, hard\n");
        scanf("%s", command);
        setgrid(command);
        ca.tgp = ca.tgp+1 ;
    }
    header();
    save();
    printf("\n");
    printGrid();
    count();

    while(M==1){
        
        printf("Enter command or 'row col num' to add value: ");
        scanf("%s", command);

        if (strcmp(command, "save") == 0) {
            saveGame();
        }
        else if (strcmp(command, "login")==0)
        {
            login();
        }
        else if (strcmp(command, "sign-up")==0)
        {
            createAccount();
        } 
        else if (strcmp(command, "exit") == 0) {
            gameExit();
            
            M = 0;
        }
        else if (strcmp(command, "instruction") == 0) {
            rules();
        } 
        else if (strcmp(command, "load-game") == 0) {
            loadGame();
        } 
        else if (strcmp(command, "deleat-game") == 0) {
            delGame();
        } 
        else if (strcmp(command, "new-game") == 0) {
            game(-1);
            return;
        } 
        else {
            row = atoi(command);
            scanf("%d %d", &col, &num);
            if (row >= 1 && row < taille+1 && col >= 1 && col < taille+1 && num > 0 && num <= 9) {
                if (isValid(row-1, col-1, num)) {
                    grid[row-1][col-1] = num;
                    header();
                    printf("\n");
                    printGrid();
                    count();
                } 
                else 
                {
                    printf("Invalid move. Try again.\n");
                    error+=1;
                    if (error==3){
                        printf ("you lost, start new game or continue witout adding score ?");
                        if (strcmp(command, "new-game") == 0) {
                            game(-1);
                            return;
                        } 
                        else if (strcmp(command, "exit") == 0) {
                            gameExit();
                            
                            M = 0;
                        }
                        else
                        {
                            score = 0;
                        }
                        
                    }
                }
            } 
            else {
                printf("Invalid input. Please enter a row, column, and value between 1-9.\n");
                printf("\n");
                printGrid();
                count();
            }
        }
    }
}

int main(){
    strcpy(ca.name , "guest");
    welcom();

    while (1)
    {
        char start[15];
        printf("Type 'start' to begin playing!\n");
        scanf("%s", start);
        if (strcmp(start, "login")==0)
        {
            login();
        }
        else if (strcmp(start, "sign-up")==0)
        {
            createAccount();
        }
        else if (strcmp(start, "load-game") == 0) {
            loadGame();
        }
        else if (strcmp(start, "instruction") == 0) {
            rules();
        }
        else if (strcmp(start, "start" ) == 0) {
            game(-1);
            return 0 ;
        } 
    }
    
    return 0;
}