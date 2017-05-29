// parallel tasks patterns

#define BEGIN_PPR
#define END_PPR

// Data are distributed, tasks replicated
#define STORE

struct ppr_task {
    long long seq;
    struct ppr_task closer;
};

#define LOCKED_GLOBAL_NEW(type, name) 

int example() {

    // the shared/replicated global state should be updated before starting
    // a new round of computation
    SYNC_DATA

    for (;;) {
        BEGIN_PPR
        
        // this is waiting for results from previous PPRs
        // several options available for passing data on to future PPRs
        // a. access to shared memory using locks (rwlocks, version locks)
        // b. access to shared memory through speculation with validation
        // c. notify later PPRs through channel messaging
        // d. pack and ship data directly to future PPRs
        // or, alternatively, have a dedicated process compute the sequential code
        
        // process inline or in subroutine
        
        END_PPR
    }


    
    // rwlock
    // declare global accumulator
    GLOBAL_NEW (double, gsum);

    for (;;) {
        BEGIN_PPR
        
        Chunk cs[] = STORE.get_chunks(df, i);
        double sum = 0;
        for (int i = cs[0].xmin; i < cs[0].imax; i++) {
            double val = STORE.doubleAt(cs[0], i);
            sum += val;
        }

        // update shared accumulate 
        wlock_acquire(gsum);
        global_value(gsum) += sum;
        wlock_release(gsum);

        END_PPR
    }

    // speculation
    for (;;) {
        BEGIN_PPR
        
        Chunk cs[] = STORE.get_chunks(df, i);
        double sum = 0;
        for (int i = cs[0].xmin; i < cs[0].imax; i++) {
            double val = STORE.doubleAt(cs[0], i);
            sum += val;
        }

        // update globals speculatively
        serial_xact_begin();
        gsum += sum;
        serial_xact_end();

        END_PPR
    }    
    
    // channel message
    Channel recv, send;
    void proc_msg() {
        // update globals if not skipped
        recv.state = READY;
    }

    recv.handle = proc_msg;
    
    for (;;) {
        BEGIN_PPR
        
        Chunk cs[] = STORE.get_chunks(df, i);
        double sum = 0;
        for (int i = cs[0].xmin; i < cs[0].imax; i++) {
            double val = STORE.doubleAt(cs[0], i);
            sum += val;
        }

        // messaging
        while (recv.state != READY) recv.wait();
        gsum += sum;
        send.notify();

        END_PPR
    }

    // pack the current work in ppr task 
    
    // task structure
    struct test1_task {
        struct ppr_task ts;
        double sum;
    };
    
    for (;;) {
        BEGIN_PPR(task1_struct)
        
        Chunk cs[] = STORE.get_chunks(df, i);
        double sum = 0;
        for (int i = cs[0].xmin; i < cs[0].imax; i++) {
            double val = STORE.doubleAt(cs[0], i);
            sum += val;
        }

        // enclose temp accumulate in the task struct
        TASK_DATA(task1_task, sum) = sum;

        END_PPR
    }

    // dedicated sequential process
    for (;;) {
        BEGIN_PPR
        
        Chunk cs[] = STORE.get_chunks(df, i);
        double sum = 0;
        for (int i = cs[0].xmin; i < cs[0].imax; i++) {
            double val = STORE.doubleAt(cs[0], i);
            sum += val;
        }

        END_PPR

        // serial code
        gsum += sum;
    }
}


