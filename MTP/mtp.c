#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

int main(void) {

    size_t in_current_idx = 0;
    size_t in_max_idx = 0;
    size_t out_current_idx = 0;

    for (;;) {
        while (in_current_idx < in_max_idx) {
            // Read data from shared buffer
            char c = input[in_current_idx++];

            // process data...
            c = c + 1;

            // write data to shared buffer
            output[out_current_idx++] = c;

            // Attempt to read the shared index, but don't block
            // if the mutex is currently locked.
            if (mutex_trylock(in_mut) == 0) {
                in_max_idx = in_shared_idx;
                mutex_unlock(in_mut);
            }
            // Attempt to write the shared index, but don't block
            // if the mutex is currently locked.
            if (mutex_trylock(out_mut) == 0) {
                out_shared_idx = out_current_idx;
                mutex_unlock(out_mut);
                cond_signal(out_cond);
            }
        }
        // We cannot continue without updating in_max_idx
        // First update shared out index for downstream thread
        mutex_lock(out_mut);
        out_shared_idx = out_current_idx;
        cond_signal(out_cond);
        mutex_unlock(out_mut);

        // Now wait for upstream thread to put data in for us
        mutex_lock(in_mut);
        in_max_idx = in_shared_idx;
        while (!(in_current_idx < in_max_idx)) {
            cond_wait(in_cond, in_mut);
            in_max_idx = in_shared_idx;
        }
        mutex_unlock(in_mut);
    }
}