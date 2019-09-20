#pragma once

#include <tensorflow/c/c_api.h>
#include <zrc.h>

typedef struct tf_brain {
	TF_Status* status;
	TF_Graph* graph;
	TF_Session* session;
	int num_input, num_output;
	TF_Output input, output;
	TF_Tensor *input_tensor;
} tf_brain_t;

void tf_brain_create(tf_brain_t *, const char *location, int num_input, int num_output);
void tf_brain_delete(tf_brain_t *);

int tf_brain_check_status(const tf_brain_t *);