#include "main.h"

pid_t *generators_pid;
pid_t *calculators_pid;
pid_t *movers_pid;
pid_t *inspectors1_pid;
pid_t *inspectors2_pid;
pid_t *inspectors3_pid;


int main(int argc, char *argv[]) {
    Config config;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }
    generators_pid = (pid_t *)malloc(config.NUM_GENERATORS * sizeof(pid_t));
    if (generators_pid == NULL) {
        perror("Error allocating memory for generators_pid");
        return 1;
    }
    // Create generator processes based on the number specified in the config file
    for (int i = 0; i < config.NUM_GENERATORS; i++) {
        pid_t generator_pid = fork();
        if (generator_pid == -1) {
            perror("Error creating generator process"); // Handle fork error
            return 1;
        } else if (generator_pid == 0) {
            // In the child process: execute the generator program
            execl("./bin/generator", "generator", argv[1], (char *)NULL);
            perror("execl failed"); // Handle execl error
            exit(1);
        }
        generators_pid[i] = generator_pid;
    }
    calculators_pid = (pid_t *)malloc(config.NUM_CALCULATORS * sizeof(pid_t));
    if (calculators_pid == NULL) {
        perror("Error allocating memory for calculators_pid");
        return 1;
    }
    // Create calculator processes based on the number specified in the config file
    for (int i = 0; i < config.NUM_CALCULATORS; i++) {
        pid_t calculator_pid = fork();
        if (calculator_pid == -1) {
            perror("Error creating calculator process"); // Handle fork error
            return 1;
        } else if (calculator_pid == 0) {
            // In the child process: execute the calculator program
            execl("./bin/calculator", "calculator", argv[1], (char *)NULL);
            perror("execl failed"); // Handle execl error
            exit(1);
        }
        calculators_pid[i] = calculator_pid;
    }
    movers_pid = (pid_t *)malloc(config.NUM_MOVERS * sizeof(pid_t));
    if (movers_pid == NULL) {
        perror("Error allocating memory for movers_pid");
        return 1;
    }

    // Create movers processes based on the number specified in the config file 
    for (int i = 0; i < config.NUM_MOVERS; i++) {
        pid_t mover_pid = fork();
        if (mover_pid == -1) {
            perror("Error creating mover process"); // Handle fork error
            return 1;
        } else if (mover_pid == 0) {
            // In the child process: execute the mover program
            execl("./bin/mover", "mover", argv[1], (char *)NULL);
            perror("execl failed"); // Handle execl error
            exit(1);
        }
        movers_pid[i] = mover_pid;
    }
    inspectors1_pid = (pid_t *)malloc(config.NUM_INSPECTOR1 * sizeof(pid_t));
    if (inspectors1_pid == NULL) {
        perror("Error allocating memory for inspectors1_pid");
        return 1;
    }
    // Create inspector1 processes based on the number specified in the config file
    for (int i = 0; i < config.NUM_INSPECTOR1; i++) {
        pid_t inspector1_pid = fork();
        if (inspector1_pid == -1) {
            perror("Error creating inspector1 process"); // Handle fork error
            return 1;
        } else if (inspector1_pid == 0) {
            // In the child process
            execl("./bin/inspector1", "inspector1", argv[1], (char *)NULL);
            perror("execl failed"); // Handle execl error
            exit(1);
        }
        inspectors1_pid[i] = inspector1_pid;
        
    }
    inspectors2_pid = (pid_t *)malloc(config.NUM_INSPECTOR2 * sizeof(pid_t));
    if (inspectors2_pid == NULL) {
        perror("Error allocating memory for inspectors2_pid");
        return 1;
    }

    // Create inspector2 processes based on the number specified in the config file
    
    for (int i = 0; i < config.NUM_INSPECTOR2; i++) {
        pid_t inspector2_pid = fork();
        if (inspector2_pid == -1) {
            perror("Error creating inspector2 process"); // Handle fork error
            return 1;
        } else if (inspector2_pid == 0) {
            // In the child process
            execl("./bin/inspector2", "inspector2", argv[1], (char *)NULL);
            perror("execl failed"); // Handle execl error
            exit(1);
        }
        inspectors2_pid[i] = inspector2_pid;
    }
    inspectors3_pid = (pid_t *)malloc(config.NUM_INSPECTOR3 * sizeof(pid_t));
    if (inspectors3_pid == NULL) {
        perror("Error allocating memory for inspectors3_pid");
        return 1;
    }

    // Create inspector3 processes based on the number specified in the config file

    for (int i = 0; i < config.NUM_INSPECTOR3; i++) {
        pid_t inspector3_pid = fork();
        if (inspector3_pid == -1) {
            perror("Error creating inspector3 process"); // Handle fork error
            return 1;
        } else if (inspector3_pid == 0) {
            // In the child process
            execl("./bin/inspector3", "inspector3", argv[1], (char *)NULL);
            perror("execl failed"); // Handle execl error
            exit(1);
        }
        inspectors3_pid[i] = inspector3_pid;
    }


    // Parent process: wait for all child processes to finish
    for (int i = 0; i < config.NUM_GENERATORS + config.NUM_CALCULATORS + config.NUM_MOVERS + config.NUM_INSPECTOR1 + config.NUM_INSPECTOR2 + config.NUM_INSPECTOR3 ; i++) {
        wait(NULL);
    }
    // Free dynamically allocated memory
    free(generators_pid);
    free(calculators_pid);
    free(movers_pid);
    free(inspectors1_pid);
    free(inspectors2_pid);
    free(inspectors3_pid);



    return 0;
}