#include <stdio.h>
#include <getopt.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <trace.h>
#include <processor.h>
#include <branch.h>
#include <unistd.h>
#include <directory.h>
#include "config.h"
#include "engine.h"

int debug = 0;
int CADSS_VERBOSE = 0;
int processorCount = 1;

void printHelp(char* prog)
{
    printf("%s \n", prog);
    printf("  -h \t Help message\n");
    printf("  -v \t Verbose\n");
    printf("  -n <num>  \t Number of processors to simulate\n");
    printf("  -c <file> \t Cache simulator\n");
    printf("  -p <file> \t Pipeline simulator\n");
    printf("  -o <file> \t Coherence simulator\n");
    printf("  -i <file> \t Interconnection simulator\n");
    printf("  -b <file> \t Branch simulator\n");
    printf("  -t <file> \t Trace file / directory\n");
    printf("  -s <file> \t Setting / configuration file\n");
}

//
// loadSim (name, type)
//    Attempts to load "name/libname.so"
//
struct sim* loadSim(char* name, char* type)
{
    char fullName[SIM_NAME_LIMIT];

    /*
     * TODO - Support for alternate naming schemes
     */
    ssize_t penUltSlash = -1;
    size_t nameLen = 0;
    do {
        if (name[nameLen] == '/')
        {
            penUltSlash = nameLen;
        }
        nameLen++;
    } while (name[nameLen] != '\0');

    if (penUltSlash != -1 && (nameLen - penUltSlash) == 1)
    {
        name[penUltSlash] = '\0';
    }

    // TODO - if debug == 1, try loading a -debug.so



    ssize_t len = snprintf(fullName, SIM_NAME_LIMIT, "%s/lib%s.so", name, name);
    if (len == SIM_NAME_LIMIT || len < 0)
    {
        fprintf(stderr, "Failed to generate so name for %s component using %s\n", type, name);

        return NULL;
    }

    void* handle = dlopen(fullName, RTLD_LAZY);
    if (handle == NULL)
    {
        fprintf(stderr, "Failed to load %s component using %s: %s\n", type, fullName, dlerror());
        return NULL;
    }

    struct sim* s = malloc(sizeof(struct sim));
    if (s == NULL)
    {
        dlclose(handle);
        fprintf(stderr, "Failed to allocate space for %s component\n", type);
        return NULL;
    }

    s->handle = handle;
    s->init = dlsym(handle, "init");
    s->tick = dlsym(handle, "tick");
    s->finish = dlsym(handle, "finish");
    s->destroy = dlsym(handle, "destroy");
    s->CADSS_VERBOSE = dlsym(handle, "CADSS_VERBOSE");
    if (s->CADSS_VERBOSE != NULL)
    {
        *(s->CADSS_VERBOSE) = CADSS_VERBOSE;
    }

    int* pCount = dlsym(handle, "processorCount");
    if (pCount != NULL)
    {
        *pCount = processorCount;
    }

    if (s->init == NULL || s->tick == NULL || s->finish == NULL || s->destroy == NULL)
    {
        dlclose(handle);
        free(s);
        fprintf(stderr, "Failed to load interface for %s component\n", type);
        return NULL;
    }
    return s;
}

int main(int argc, char** argv)
{
    int opt;
    struct sim* csim = NULL;
    struct sim* psim = NULL;
    struct sim* bsim = NULL;
    struct sim* isim = NULL;
    struct sim* osim = NULL;
    struct sim* trace = NULL;
    struct sim* dsim = NULL;
    char* settingFile = NULL;
    char* cacheName = NULL;
    char* branchName = NULL;
    char* procName = NULL;
    char* coherName = NULL;
    char* interName = NULL;

    // TODO - switch to getopt_long that accepts -- arguments
    while ((opt = getopt(argc, argv, "hdvc:p:o:n:i:b:t:s:")) != -1)
    {
        switch(opt)
        {
            case 'd':
                debug = 1;
                break;
            case 'h':
                printHelp(argv[0]);
                return 0;
            case 'p':
                procName = optarg;
                break;
            case 'v':
                CADSS_VERBOSE = 1;
                break;
            case 'c':
                cacheName = optarg;
                break;
            case 'b':
                branchName = optarg;
                break;
            case 's':
                settingFile = optarg;
                break;
            case 'o':
                coherName = optarg;
                break;
            case 'n':
                processorCount = atoi(optarg);
                break;
            case 'i':
                interName = optarg;
                break;
        }
    }

    trace = loadSim("trace", "trace");
    trace_sim_args tsa;
    tsa.arg_count = argc;
    tsa.arg_list = argv;
    optind = 1;
    trace_reader* tr = trace->init(&tsa);

    if (settingFile == NULL)
    {
        fprintf(stderr, "No setting file specified, using default.config\n");
        settingFile = "default.config";
    }
    if (openSettings(settingFile) != 0)
    {
        fprintf(stderr, "Failed to open setting file - %s\n", settingFile);
        return 0;
    }

    if (interName == NULL)
    {
        isim = loadSim("interconnect", "interconnect");
    }
    else
    {
        isim = loadSim(interName, "interconnect");
        if (isim == NULL)
        {
            return 0;
        }
    }
    // Loading directory sim. Not how this works and if it will :/
    dsim = loadSim("directory", "directory");

    if (coherName == NULL)
    {
        osim = loadSim("coherence", "coherence");
    }
    else
    {
        osim = loadSim(coherName, "coherence");
        if (osim == NULL)
        {
            return 0;
        }
    }

    if (cacheName == NULL)
    {
        csim = loadSim("cache", "cache");
    }
    else
    {
        csim = loadSim(cacheName, "cache");
        if (csim == NULL)
        {
            return 0;
        }
    }

    if (procName == NULL)
    {
        psim = loadSim("processor", "processor");
    }
    else
    {
        psim = loadSim(procName, "processor");
        if (psim == NULL)
        {
            return 0;
        }
    }

    if (branchName == NULL)
    {
        bsim = loadSim("branch", "branch");
    }
    else
    {
        bsim = loadSim(branchName, "branch");
        if (bsim == NULL)
        {
            return 0;
        }
    }

    // B.N. - set optind to 1 before calling init on any component
    //  that resets getopt() so the component can use it safely on its arguments
    int argCount = 0;
    char** arg = NULL;

    cache* cache_sim = NULL;
    branch* branch_sim = NULL;
    coher* coher_sim = NULL;
    interconn* inter_sim = NULL;
    direc* direct_sim = NULL;


    arg = getSettings("interconnect", &argCount);
    if (arg = NULL)
    {

    }

    inter_sim_args isa;
    isa.arg_count = argCount;
    isa.arg_list = arg;
    optind = 1;
    if ((inter_sim = isim->init(&isa)) == NULL)
    {

    }

    // Add directory
    arg = getSettings("directory", &argCount);
    if (arg = NULL)
    {

    }
    direc_sim_args dsa;
    dsa.arg_count = argCount;
    dsa.arg_list = arg;
    dsa.inter = inter_sim;
    optind = 1;
    if ((direct_sim = dsim->init(&dsa)) == NULL)
    {

    }

    arg = getSettings("coherence", &argCount);
    if (arg == NULL)
    {

    }

    coher_sim_args osa;
    osa.arg_count = argCount;
    osa.arg_list = arg;
    osa.inter = inter_sim;
    osa.direct = direct_sim;
    optind = 1;
    if ((coher_sim = osim->init(&osa)) == NULL)
    {

    }

    optind = 1;
    arg = getSettings("cache", &argCount);
    if (arg == NULL)
    {

    }

    cache_sim_args csa;
    csa.arg_count = argCount;
    csa.arg_list = arg;
    csa.coherComp = coher_sim;
    if ((cache_sim = csim->init(&csa)) == NULL)
    {

    }

    optind = 1;
    arg = getSettings("branch", &argCount);
    if (arg == NULL)
    {

    }

    branch_sim_args bsa;
    bsa.arg_count = argCount;
    bsa.arg_list = arg;
    if ((branch_sim = bsim->init(&bsa)) == NULL)
    {

    }

    optind = 1;
    arg = getSettings("processor", &argCount);
    if (arg == NULL)
    {

    }

    processor_sim_args psa;
    psa.arg_count = argCount;
    psa.arg_list = arg;
    psa.tr = tr;
    psa.cache_sim = cache_sim;
    psa.branch_sim = branch_sim;
    if (psim->init(&psa) != 0)
    {

    }


    // Main sim loop
    int progress = 0;
    do {
        // Processor requests trace ops as needed
        progress = psim->tick();
    } while (progress);

    psim->finish(STDOUT_FILENO);
    psim->destroy();
    trace->destroy();

    dlclose(csim->handle);
    free(csim);
    dlclose(psim->handle);
    free(psim);
    dlclose(bsim->handle);
    free(bsim);
    dlclose(trace->handle);
    free(trace);

    return 0;
}
