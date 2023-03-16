#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

FILE *fp;

typedef struct Data
{
    int** arr1;
    int** arr2;
    int** res;
    int currRow;
    int currCol;
}Data;



int** readingInput(const char* fileName){
    char* line =calloc(strlen(fileName)+5, sizeof(char));
    strncpy(line, fileName, strlen(fileName));
    strncat(line, ".txt", 4);

    size_t len = 0;
    int rows, cols;

    fp = fopen(line, "r");
    if(fp == NULL){
        printf("Error opening file %s\n", line);
        return NULL;
    }
    
    fscanf(fp, "row=%d col=%d", &rows, &cols);

    // if(fscanf(fp, "row=%d col=%d", &rows, &cols) != 1){
    //     printf("Error reading rows and cols from %s\n", line);
    //     return NULL;
    // }


    int **result = calloc(rows + 1, sizeof(int *));
    result[0] = calloc(2, sizeof(int));;
    result[0][0] = rows;
    result[0][1] = cols;
    for (int i = 0; i < rows; i++) {
        result[i+1] = calloc(cols, sizeof(int));
        for (int j = 0; j < cols; j++) {
            if(fscanf(fp, "%d", &result[i+1][j]) != 1){
                printf("Error reading file\n");
                return NULL;
            }
        }
    }

    free(line);
    fclose(fp);
    return result;
}




void mul_per_matrix(int** arr1, int** arr2, char* fileName){
    int rows=arr1[0][0], cols=arr2[0][1], rows2=arr2[0][0];
    int** res = calloc(rows, sizeof(int*));
    for(int i=0;i<rows;i++){
        res[i] = calloc(cols, sizeof(int));
        for(int j=0;j<cols;j++){
            int sum = 0;
            for(int h=0;h<rows2;h++){
                sum += arr1[i+1][h]*arr2[h+1][j];
            }
            res[i][j] = sum;
        }
    }

    if(fileName == NULL){
        fp = fopen("c_per_matrix.txt", "w");
    }else{
        strncat(fileName, "_per_matrix.txt", 16);
        fp = fopen(fileName, "w");
    }
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    fprintf(fp, "Method: A thread per matrix\n");
    fprintf(fp, "row=%d col=%d \n", rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(fp, "%d ", res[i][j]);
        }
        fprintf(fp, "\n");
    }

    for(int i=0;i<rows;i++){
        free(res[i]);
    }
    free(res);
    
    fclose(fp);
    printf("Per matrix method:\nNumber of threads used = 1\n");
}





void* mul_single_row(void* data){
    Data* temp = (Data *)data;
    for(int j=0;j<temp->arr2[0][1];j++){
        int sum = 0;
        for(int h=0;h<temp->arr2[0][0];h++){
            sum += temp->arr1[temp->currRow+1][h]*temp->arr2[h+1][j];
        }
        temp->res[temp->currRow][j] = sum;
    }

    pthread_exit(0);
}



void mul_per_row(int** arr1, int** arr2, char* fileName){
    int rows = arr1[0][0], cols = arr2[0][1];
    int** res = calloc(rows, sizeof(int*));
    for(int i=0;i<rows;i++){
        res[i] = calloc(cols, sizeof(int));
    }
    pthread_t threads[rows];
    Data data[rows];


    for(int i=0;i<rows;i++){
        data[i].arr1 = arr1;
        data[i].arr2 = arr2;
        data[i].res  = res;
        data[i].currRow = i;
        pthread_create(&threads[i], NULL, mul_single_row, (void*)&data[i]);
    }

    
    for (int i=0;i<rows;i++) {
        pthread_join(threads[i], NULL);
    }

    if(fileName == NULL){
        fp = fopen("c_per_row.txt", "w");
    }else{
        strncat(fileName, "_per_row.txt", 13);
        fp = fopen(fileName, "w");
    }
    if (fp == NULL) {-
        printf("Error opening file.\n");
        return;
    }

    fprintf(fp, "Method: A thread per row\n");
    fprintf(fp, "row=%d col=%d \n", rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(fp, "%d ", res[i][j]);
        }
        fprintf(fp, "\n");
    }

    for(int i=0;i<rows;i++){
        free(res[i]);
    }

    free(res);
    fclose(fp);
    printf("Per row method:\nNumber of threads used = %d\n", rows);
}





void* mul_single_element(void* data){
    Data* temp = (Data *)data;
    int sum = 0;
    for(int h=0;h<temp->arr2[0][0];h++){
        sum += temp->arr1[temp->currRow+1][h]*temp->arr2[h+1][temp->currCol];
    }
    temp->res[temp->currRow][temp->currCol] = sum;

    data = NULL;
    temp = NULL;
    pthread_exit(0);
}


void mul_per_element(int** arr1, int** arr2, char* fileName){
    int rows = arr1[0][0], cols = arr2[0][1];
    int** res = calloc(rows, sizeof(int*));
    for(int i=0;i<rows;i++){
        res[i] = calloc(cols, sizeof(int));
    }

    pthread_t threads[rows][cols];
    Data data[rows][cols];


    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            data[i][j].arr1 = arr1;
            data[i][j].arr2 = arr2;
            data[i][j].res  = res;
            data[i][j].currRow = i;
            data[i][j].currCol = j;
            pthread_create(&threads[i][j], NULL, mul_single_element, (void*)&data[i][j]);
        }
    }

    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            pthread_join(threads[i][j], NULL);
        }
    }



    if(fileName == NULL){
        fp = fopen("c_per_element.txt", "w");
    }else{
        strncat(fileName, "_per_element.txt", 16);
        fp = fopen(fileName, "w");
    }
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    fprintf(fp, "Method: A thread per element\n");
    fprintf(fp, "row=%d col=%d \n", rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(fp, "%d ", res[i][j]);
        }
        fprintf(fp, "\n");
    }

    for(int i=0;i<rows;i++){
        free(res[i]);
    }
    free(res);
    fclose(fp);
    printf("Per row method:\nNumber of threads used = %d\n", rows*cols);
}




int main(int argc, char const *argv[])
{

    if((argc != 1) && (argc != 4)){
        printf("Wrong parameters\n");
    }

    int** arr1, **arr2;
    if(argv[1] == NULL){
        arr1 = readingInput("a");
        arr2 = readingInput("b");
    }else{
        arr1 = readingInput(argv[1]);
        arr2 = readingInput(argv[2]);
    }

    if(arr1 == NULL || arr2 == NULL){
        return 0;
    }

    struct timeval stop, start;

    char* des;

    if(argc == 4){
        des = calloc(strlen(argv[3])+18, sizeof(char));
        strncpy(des, argv[3], strlen(argv[3]));
    }else{
        des = NULL;
    }


    gettimeofday(&start, NULL); //start checking time
    mul_per_matrix(arr1, arr2, des);
    gettimeofday(&stop, NULL); //end checking time
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);

    if(argc == 4){
        free(des);
        des = calloc(strlen(argv[3])+18, sizeof(char));
        strncpy(des, argv[3], strlen(argv[3]));
    }else{
        des = NULL;
    }

    gettimeofday(&start, NULL); //start checking time
    mul_per_row(arr1, arr2, des);
    gettimeofday(&stop, NULL); //end checking time
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);

    if(argc == 4){
        free(des);
        des = calloc(strlen(argv[3])+18, sizeof(char));
        strncpy(des, argv[3], strlen(argv[3]));
    }else{
        des = NULL;
    }

    gettimeofday(&start, NULL); //start checking time
    mul_per_element(arr1, arr2, des);
    gettimeofday(&stop, NULL); //end checking time
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);


    int size = arr1[0][0];
    for(int i=0;i<size+1;i++){
        free(arr1[i]);
    }
    free(arr1);

    size = arr2[0][0];
    for(int i=0;i<size+1;i++){
        free(arr2[i]);
    }
    free(arr2);

    free(des);

    return 0;
}
