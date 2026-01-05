#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/*
This program is supposed to be a basic finite element model of 1D
conduction in a rod.
*/

#define NUM_SEGS 32         // Number of elements [-]
#define COND 200.0          // Thermal conductivity [W/(m*K)]
#define CP 376.8            // Specific heat capacity [J/(kg*K)]
#define RHO 8940.0          // Mass density [kg/m^3]
#define DIAM (25.0/1000.0)  // Diameter of the rod [m]
#define LENGTH 1.0          // Length of the rod [m]
#define DT 1.0              // Time step [s]
#define TIME 4*3600.0         // Simulation time [s]
#define PI (355.0/113.0)

void delay(int num_seconds);

int main(){
    float area_cs = PI*DIAM*DIAM/4;
    float len_seg = LENGTH/(float)NUM_SEGS;
    float vol_seg = area_cs*len_seg;
    float mass_seg = vol_seg*RHO;
    float c_seg = mass_seg*CP;
    float T[NUM_SEGS], C[NUM_SEGS], Q[NUM_SEGS], dT_dt[NUM_SEGS];
    float time_elapsed = 0.0;
    int forcing_loc = 0;

    float ic_value, forcing_value;
    char forcing_type;

    FILE *fp = fopen("results.csv", "w");
    fprintf(fp, "time");
    for(int ii = 0; ii < NUM_SEGS; ii++){
        fprintf(fp, ",T%d", ii);
    }
    fprintf(fp, "\n");

    printf("Input the initial temperature in Kelvin: ");
    scanf("%f", &ic_value);
    if((ic_value < 193.15) || (ic_value > 1273.15)){    // No silly temperatures, please
        printf("\nInvalid temperature: please input a value between 193.15 and 1273.15 K");
        exit(EXIT_FAILURE);
    }
    getchar();
    printf("\nInput the type of forcing to apply ('T' or 'Q'): ");
    forcing_type = getchar();
    if ((forcing_type != 'T') && (forcing_type != 'Q')){    
        printf("\nInvalid forcing type: please select either 'T' or 'Q'\n");
        exit(EXIT_FAILURE);
    }
    getchar();
    printf("\nInput the segment index to apply forcing to: ");
    char loc_input = getchar();
    while(loc_input != '\n'){
        if ((loc_input - '0') < 0 || (loc_input - '0') > 9){
            printf("\nForcing location only accepts integer values\n");
            exit(EXIT_FAILURE);
        }
        forcing_loc = 10*forcing_loc + (loc_input - '0');
        loc_input = getchar();
    }
    if (forcing_loc < 0 || forcing_loc >= NUM_SEGS){
        printf("\nForcing location out of range: please do not exceed segment count\n");
        exit(EXIT_FAILURE);
    }

    printf("\nInput the forcing value (watts or temperature): ");
    scanf("%f", &forcing_value);


    for(int ii = 0; ii < NUM_SEGS; ii++){
        C[ii] = c_seg;
        T[ii] = ic_value;
        dT_dt[ii] = 0.0;
    }

    while(time_elapsed < TIME){

        for(int ii = 0; ii < NUM_SEGS; ii++){
            printf(" %f ", T[ii]);
            if(ii == 0){
                Q[ii] = -COND*area_cs*(T[ii] - T[ii+1])/len_seg;
            } else if(ii == (NUM_SEGS - 1)){
                Q[ii] = -COND*area_cs*(T[ii] - T[ii-1])/len_seg;
            } else {
                Q[ii] = -COND*area_cs*((T[ii] - T[ii+1]) + (T[ii] - T[ii-1]))/len_seg;
            }   
        }
        if(forcing_type == 'Q'){
            Q[forcing_loc] += forcing_value;  // Add heat source to conduction
        }
        for(int ii = 0; ii < NUM_SEGS; ii++){
            dT_dt[ii] = Q[ii] / C[ii];
            T[ii] += dT_dt[ii]*DT;
        }
        if(forcing_type == 'T'){
            T[(int)forcing_loc] = forcing_value;
        }
        printf("K\n %f seconds \n", time_elapsed);
        time_elapsed = time_elapsed + DT;
        fprintf(fp, "%f", time_elapsed);
        for(int ii = 0; ii < NUM_SEGS; ii++){
            fprintf(fp, ",%f", T[ii]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    exit(EXIT_SUCCESS);
    
}

void delay(int num_seconds){
    time_t start_time = time(NULL);
    while(time(NULL) < start_time + num_seconds)
        ;
}