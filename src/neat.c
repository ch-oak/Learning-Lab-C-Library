#include "llab.h"


neat* init(int max_buffer, int input, int output){
    /*allocation*/
    int i,j,z,k,w,flag,min,max,total_species = 0,count = 0;
    int global_inn_numb_connections,global_inn_numb_nodes, actual_genomes = INITIAL_POPULATION, n_survivors,temp_gg2_counter = 0,temp_gg3_counter = 0, n_species;
    int* dict_connections;
    int** matrix_nodes;
    int** matrix_connections;
    species* s;
    genome* g;
    genome** gg;
    genome** temp_gg1;
    genome** temp_gg2;
    genome** temp_gg3;
    float a,b,n;
    
    
    /*initialization empty genome*/
    g = init_genome(input,output);
    
    /*initializing global params*/
    init_global_params(input,output,&global_inn_numb_nodes,&global_inn_numb_connections,&dict_connections,&matrix_nodes,&matrix_connections);

    
    /*filling the gg list with init genome, we allocate a big space for gg.
     * gg is a list filled with genomes for each generation, this number of genomes
     * could vary during the generations*/
    gg = (genome**)malloc(sizeof(genome*)*INITIAL_POPULATION*max_buffer);
    temp_gg2 = (genome**)malloc(sizeof(genome*)*INITIAL_POPULATION*max_buffer);//is used for crossover
    temp_gg3 = (genome**)malloc(sizeof(genome*)*INITIAL_POPULATION*max_buffer);//is used to save the rapresentative genomes for each generation
    
    gg[0] = copy_genome(g);
    free_genome(g,global_inn_numb_connections);

    /*initialize first specie with a single rapresentative genome (the empty genome)*/
    s = create_species(gg,1,global_inn_numb_connections,SPECIES_THERESHOLD,&total_species);
    
    
    for(i = 1; i < INITIAL_POPULATION; i++){
        gg[i] = copy_genome(gg[0]);
        add_random_connection(gg[i],&global_inn_numb_connections,&matrix_connections,&dict_connections);
        
    }
    count+=actual_genomes;
    
    neat* nes = (neat*)malloc(sizeof(neat));
    nes->total_species = total_species;
    nes->count = count;
    nes->actual_genomes = actual_genomes;
    nes->global_inn_numb_connections = global_inn_numb_connections;
    nes->global_inn_numb_nodes= global_inn_numb_nodes;
    nes->matrix_connections = matrix_connections;
    nes->matrix_nodes = matrix_nodes;
    nes->dict_connections = dict_connections;
    nes->gg = gg;
    nes->s = s;
    nes->temp_gg2 = temp_gg2;
    nes->temp_gg3 = temp_gg3;
    nes->g = NULL;
    return nes;
}
void neat_generation_run(neat* nes, genome** gg){

    //save best genome
    nes->n = -1;
    for(nes->i = 0; nes->i < nes->actual_genomes; nes->i++){
        if(gg[nes->i]->fitness > nes->n){
            nes->j = nes->i;
            nes->n = gg[nes->i]->fitness;
        }
        
    }
    free(nes->g);
    nes->g = copy_genome(gg[nes->j]);

    if(nes->k%SAVING == 0 || nes->k == GENERATIONS)
        save_genome(gg[nes->j],nes->global_inn_numb_connections,nes->k+1);

    if(nes->k == GENERATIONS)
    return;


    /*speciation*/
    nes->s = put_genome_in_species(gg,nes->actual_genomes,nes->global_inn_numb_connections,SPECIES_THERESHOLD,&nes->total_species,&nes->s);


    /* we copied the genomes in species, now deallocate the genomes in gg */
    for(nes->i = 0; nes->i < nes->actual_genomes; nes->i++){
        free_genome(gg[nes->i],nes->global_inn_numb_connections);
    }
    nes->max = -1;
    nes->sum = 0;
    nes->z = 0;
    for(nes->i = 0; nes->i < nes->total_species; nes->i++){
        if(nes->s[nes->i].numb_all_other_genomes > 0){
            nes->z++;
            nes->sum+=nes->s[nes->i].numb_all_other_genomes;
            if(nes->s[nes->i].numb_all_other_genomes > nes->max)
                nes->max = nes->s[nes->i].numb_all_other_genomes;
        }
    }

    nes->n_species = nes->z;
    nes->sum/=(float)nes->n_species;
    nes->a = get_mean_fitness(nes->s,nes->total_species);

    nes->actual_genomes = 0;nes->temp_gg2_counter = 0; nes->temp_gg3_counter = 0;

    for(nes->i = 0; nes->i < nes->total_species; nes->i++){
        /*compute mean fitnesses of species*/
        if(nes->s[nes->i].numb_all_other_genomes > 0){
            nes->b = get_mean_specie_fitness(nes->s,nes->i);
            nes->b/=nes->a;
            nes->temp_gg1 = sort_genomes_by_fitness(nes->s[nes->i].all_other_genomes,nes->s[nes->i].numb_all_other_genomes);
            /*if a specie didn't improve its for at least 15 generations we kill that specie except in the case where the number of speicies are few*/
            if(nes->s[nes->i].rapresentative_genome->specie_rip < 15 || nes->n_species < 10){
                /*b >= 1 means the mean fintess of this specie is above the mean fitness of the population
                 * in that case or in the case in which the best fitness of the specie doesn't improve we incremant the rip counter*/
                if(nes->temp_gg1[0]->fitness <= nes->s[nes->i].rapresentative_genome->fitness || nes->b < 1)
                    nes->s[nes->i].rapresentative_genome->specie_rip++;
                else if(nes->temp_gg1[0]->fitness > nes->s[nes->i].rapresentative_genome->fitness){
                    nes->s[nes->i].rapresentative_genome->fitness = nes->temp_gg1[0]->fitness; 
                    nes->s[nes->i].rapresentative_genome->specie_rip=0;
                }
                else
                    nes->s[nes->i].rapresentative_genome->specie_rip=0;
                /*in temp_gg3 we save the rapresentative genome of this specie*/
                    
                nes->temp_gg3[nes->temp_gg3_counter] = copy_genome(nes->s[nes->i].rapresentative_genome);    
                /*in temp_gg2 we save the best genome of this specie*/
                if(nes->b >= 1){
                    if(nes->s[nes->i].numb_all_other_genomes>1){
                        nes->temp_gg2[nes->temp_gg2_counter] = copy_genome(nes->temp_gg1[0]);
                        nes->temp_gg2_counter++;
                        nes->temp_gg2[nes->temp_gg2_counter] = copy_genome(nes->temp_gg1[1]);
                        nes->temp_gg2_counter++;
                    }
                }
                
                nes->temp_gg3_counter++;
                double bb = round_up(nes->b*3.67);
                for(nes->z = 0; nes->z < (CHILDREN*(1+bb)); nes->z+=round_up(nes->s[nes->i].numb_all_other_genomes*PERCENTAGE_SURVIVORS_PER_SPECIE)){
                    for(nes->w = 0; nes->w < nes->s[nes->i].numb_all_other_genomes; nes->w++){
                        if(nes->w >= round_up(nes->s[nes->i].numb_all_other_genomes*PERCENTAGE_SURVIVORS_PER_SPECIE)){
                            break;
                        }
                        gg[nes->actual_genomes] = copy_genome(nes->temp_gg1[nes->w]);
                        /*mutations*/
                        activate_connections(gg[nes->actual_genomes],nes->global_inn_numb_connections,ACTIVATE_CONNECTION_RATE);
                        connections_mutation(gg[nes->actual_genomes],nes->global_inn_numb_connections, CONNECTION_MUTATION_RATE,NEW_CONNECTION_ASSIGMENT_RATE);
                        
                        /*big species*/
                        if(nes->s[nes->i].numb_all_other_genomes >= nes->sum){
                            
                            if(r2() < ADD_CONNECTION_BIG_SPECIE_RATE){
                                add_random_connection(gg[nes->actual_genomes],&nes->global_inn_numb_connections,&nes->matrix_connections,&nes->dict_connections);
                            }
                            
                            else if(r2() < REMOVE_CONNECTION_RATE){
                                remove_random_connection(gg[nes->actual_genomes],nes->global_inn_numb_connections);
                            }
                        }
                        
                        /*small specie*/
                        else{
                            if(r2() < ADD_CONNECTION_SMALL_SPECIE_RATE){
                                add_random_connection(gg[nes->actual_genomes],&nes->global_inn_numb_connections,&nes->matrix_connections,&nes->dict_connections);
                            }
                            
                            else if(r2() < REMOVE_CONNECTION_RATE){
                                remove_random_connection(gg[nes->actual_genomes],nes->global_inn_numb_connections);
                            }
                        }
                                
                        if(r2() < ADD_NODE_SPECIE_RATE)
                            split_random_connection(gg[nes->actual_genomes],&nes->global_inn_numb_nodes,&nes->global_inn_numb_connections,&nes->dict_connections,&nes->matrix_nodes,&nes->matrix_connections);
                        
                        
                        
                        nes->actual_genomes++;    
                    }
                }
            }
            free(nes->temp_gg1);
            
        }
        
    }

    for(nes->i = 0; nes->i < nes->actual_genomes; nes->i++){
        gg[nes->i]->fitness = 0;
    }
            
    //these lines save for the next generations the best genomes of the surviving species too
        //but the tests show that is better keeping disabled these lines

    //for(nes->i = 0; nes->i < nes->temp_gg2_counter; nes->i++){
        //gg[nes->actual_genomes] = copy_genome(nes->temp_gg2[nes->i]);
        //nes->actual_genomes++;
    //}





    free_species(nes->s,nes->total_species,nes->global_inn_numb_connections);
    nes->total_species = 0;
    nes->s = create_species(nes->temp_gg3,nes->temp_gg3_counter,nes->global_inn_numb_connections,SPECIES_THERESHOLD,&nes->total_species);

    for(nes->i = 0; nes->i < nes->temp_gg3_counter; nes->i++){
        free_genome(nes->temp_gg3[nes->i],nes->global_inn_numb_connections);
    }

    for(nes->i = 0; nes->i < nes->temp_gg2_counter; nes->i++){
        nes->temp_gg3[nes->i] = copy_genome(nes->temp_gg2[nes->i]);
    }

    for(nes->i = 0; nes->i < nes->temp_gg2_counter-1; nes->i+=2){
            if(r2() < CROSSOVER_RATE){
                gg[nes->actual_genomes] = crossover(nes->temp_gg2[nes->i],nes->temp_gg2[nes->i+1],nes->global_inn_numb_connections,nes->global_inn_numb_nodes);
                gg[nes->actual_genomes]->fitness = 0;
                nes->actual_genomes++;
            }
                
        
    }

    for(nes->i = 0; nes->i < nes->temp_gg2_counter; nes->i++){
        free_genome(nes->temp_gg2[nes->i],nes->global_inn_numb_connections);
        free_genome(nes->temp_gg3[nes->i],nes->global_inn_numb_connections);
    }

    gg[nes->actual_genomes] = copy_genome(nes->g);
    nes->actual_genomes++;
    free_genome(nes->g,nes->global_inn_numb_connections);
    nes->g = NULL;
    nes->count+=nes->actual_genomes;
}

void free_neat(neat* nes){
    free_species(nes->s,nes->total_species,nes->global_inn_numb_connections);
    free(nes->temp_gg2);
    free(nes->temp_gg3);
    
    for(nes->i = 0; nes->i < nes->actual_genomes; nes->i++){
        free_genome(nes->gg[nes->i],nes->global_inn_numb_connections);
    }
    for(nes->i = 0; nes->i < nes->global_inn_numb_nodes; nes->i++){
        free(nes->matrix_nodes[nes->i]);
    }
    
    for(nes->i = 0; nes->i < nes->global_inn_numb_connections; nes->i++){
        free(nes->matrix_connections[nes->i]);
    }
    if(nes->g!= NULL)
    free_genome(nes->g,nes->global_inn_numb_connections);
    free(nes->gg);
    free(nes->matrix_nodes);
    free(nes->matrix_connections);
    free(nes->dict_connections);
    free(nes);
}
