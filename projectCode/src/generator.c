
#include "generator.h"

float getRandomFloat(int min, int max);
void generateCSV();
void createDirectory(const char *path);
int getRandomInt(int min, int max);


Config config;


int main(int argc, char *argv[])
{
    //check num of arguments
    if (argc != 2)
    {
        perror("Invalid number of arguments");
        return 1;
    }


    // load config file
    if (load_config(argv[1], &config) == -1) {
        perror("Error loading configuration");
        return 1;
    }

    srand(time(NULL)+getpid());

    createDirectory(homeDir);
    generateCSV();




return 0;


}

void createDirectory(const char *path)
{
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0777);
    }
}


float getRandomFloat(int min, int max)
{


    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
    // code here
}

int getRandomInt(int min, int max)
{

    return rand() % (max - min + 1) + min;
}

void generateCSV()
{
    int i = 0;
    char * HOME_DIR = homeDir;
    char filename[100];

    int Rows = 10000, Cols = 10;
    int minValue = -100, maxValue = 100;
    // int Rows = 3, Cols = 2;

     sprintf(filename, "%s/%d.csv", HOME_DIR, i);

        FILE *file = fopen(filename, "w");
        if (!file) {
            perror("File creation failed");
            exit(1);
        }

        if (!(config.MIN_ROW ==-1 || config.MAX_ROW ==-1))
        {
            Rows = getRandomInt(config.MIN_ROW, config.MAX_ROW);
           
        }

        if(!(config.MIN_COLUMN ==-1 || config.MAX_COLUMN ==-1))
        {
            Cols = getRandomInt(config.MIN_COLUMN, config.MAX_COLUMN);
        }

        if(!(config.MIN_VALUE ==-1 || config.MAX_VALUE ==-1))
        {
            minValue = config.MIN_VALUE;
            maxValue = config.MAX_VALUE;
        }

        printf("\033[0;34mRows: %d, Cols: %d, MinValue: %d, MaxValue: %d\033[0m\n", Rows, Cols, minValue, maxValue);
        //print missing percentage
        printf("\033[0;34mMissing percentage: %f\033[0m\n", config.MISS_PERCENTAGE);



 
        

        for (int i = 0; i < Rows; i++)
        {
            for (int j = 0; j < Cols; j++)
            {

                if (getRandomFloat(0, 1) < config.MISS_PERCENTAGE)// missing value
                {
                    if (j < Cols - 1)
                     fprintf(file, " ,");
 
                }
                else
                {
                    fprintf(file, "%f", getRandomFloat(minValue, maxValue)); // random value
                    // fprintf(file, "%f", getRandomFloat(3, 15)); // random value
                    if (j < Cols - 1)
                    {
                        fprintf(file, ",");
                    }
                }


            }
            fprintf(file, "\n");
        }

        fclose(file);

        printf("\033[0;32mCSV file: %s generated successfully\033[0m\n", filename);




}
