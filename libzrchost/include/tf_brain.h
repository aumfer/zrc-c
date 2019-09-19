#pragma once

#include <tensorflow/c/c_api.h>
#include <zrc.h>

typedef struct tf_brain {
	TF_Status* status;
	TF_Graph* graph;
	TF_Session* session;
	TF_Output input, output;
	TF_Tensor *input_tensor;
	TF_Tensor *output_tensor;
} tf_brain_t;

void tf_brain_create(tf_brain_t *);
void tf_brain_delete(tf_brain_t *);

void tf_brain_update(tf_brain_t *, zrc_t *);