#ifndef PARALLEL_INPUT_H
#define PARALLEL_INPUT_H

int parallel_input_init();
void * parallel_input_routine(void * cookie);
void parallel_input_deinit();

#endif /* PARALLEL_INPUT_H */