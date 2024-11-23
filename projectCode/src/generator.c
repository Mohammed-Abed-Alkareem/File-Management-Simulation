
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

float getRandomFloat(int min, int max);
void generateCSV();
void createDirectory(const char *path);
int getRandomInt(int min, int max);


// Config config;


int main(argc, argv)
{
    // //check num of arguments
    // if (argc != 2)
    // {
    //     perror("Invalid number of arguments");
    //     return 1;
    // }


    // // load config file
    // if (load_config(argv[1], &config) == -1) {
    //     perror("Error loading configuration");
    //     return 1;
    // }

    createDirectory("./data");
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
    char * HOME_DIR = "./data";
    char filename[100];

    // int Rows = 10000, Cols = 10;
    int Rows = 3, Cols = 2;

     sprintf(filename, "%s/%d.csv", HOME_DIR, i);

        FILE *file = fopen(filename, "w");
        if (!file) {
            perror("File creation failed");
            exit(1);
        }

        // if (!(config.MIN_ROWS ==-1 || config.MAX_ROWS ==-1 || config.MIN_COLS ==-1 || config.MAX_COLS ==-1 || config.MIN_VAL ==-1 || config.MAX_VAL ==-1))
        // {
        //     Rows = getRandomInt(config.MIN_ROWS, config.MAX_ROWS);
        //     Cols = getRandomInt(config.MIN_COLS, config.MAX_COLS);
        // }

      // there is a chance of missing
        srand(time(NULL)+getpid());

        for (int i = 0; i < Rows; i++)
        {
            for (int j = 0; j < Cols; j++)
            {

                // if (getRandomFloat(0, 1) < config.MISSING_PROB)// missing value
                if (getRandomFloat(0, 1) < 0.1)// missing value
                {
                    if (j < Cols - 1)
                    {
                        fprintf(file, ",");
                    }
                }
                else
                {
                    // fprintf(file, "%f,", getRandomFloat(config.MIN_VAL, config.MAX_VAL)); // random value
                    fprintf(file, "%f", getRandomFloat(3, 15)); // random value
                    if (j < Cols - 1)
                    {
                        fprintf(file, ",");
                    }
                }


            }
            fprintf(file, "\n");
        }

        fclose(file);




}
