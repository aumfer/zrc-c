#include <tf_brain.h>
#include <stdio.h>
#include <io.h>

void tf_brain_create(tf_brain_t *tf_brain, const char *location, int num_input, int num_output) {
	//TF_RegisterLogListener(puts);
	tf_brain->num_input = num_input;
	tf_brain->num_output = num_output;
	tf_brain->status = TF_NewStatus();
	tf_brain->graph = TF_NewGraph();
	tf_brain->input_tensor = TF_AllocateTensor(TF_FLOAT, (int64_t[]) { 1, num_input }, 2, num_input * sizeof(float));

	char file[_MAX_PATH];
	sprintf_s(file, sizeof(file), "%ssaved_model.pb", location);
	if (_access(file, 0) == -1) {
		return;
	}

#if 1
	TF_SessionOptions* session_options = TF_NewSessionOptions();
	tf_brain->session = TF_LoadSessionFromSavedModel(session_options, 0, location, (char*[]) { "serve" }, 1, tf_brain->graph, 0, tf_brain->status);
	tf_brain_check_status(tf_brain);
	TF_DeleteSessionOptions(session_options);
#else
	TF_SessionOptions* session_options = TF_NewSessionOptions();
	tf_brain->session = TF_NewSession(tf_brain->graph, session_options, tf_brain->status);
	tf_brain_check_status(tf_brain);
	TF_DeleteSessionOptions(session_options);
	TF_ImportGraphDefOptions* import_options = TF_NewImportGraphDefOptions();
	//FILE *f = fopen("C:\\GitHub\\aumfer\\zrc-learn\\test.pb", "rb");
	FILE *f = fopen("C:\\GitHub\\openai\\spinningup\\log\\simple_save\\saved_model.pb", "rb");
	printf("%d", errno);
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	void *buf = malloc(len);
	fseek(f, 0, SEEK_SET);
	size_t read = fread(buf, 1, len, f);
	zrc_assert(read == len);
	TF_Buffer* graph_def = TF_NewBufferFromString(buf, len);
	free(buf);
	fclose(f);
	TF_GraphImportGraphDef(tf_brain->graph, graph_def, import_options, tf_brain->status);
	tf_brain_check_status(tf_brain);
	TF_DeleteImportGraphDefOptions(import_options);
#endif
#if 0
	size_t pos = 0;
	TF_Operation *op;
	while (op = TF_GraphNextOperation(tf_brain->graph, &pos)) {
		const char *name = TF_OperationName(op);
		const char *type = TF_OperationOpType(op);
		int num_inputs = TF_OperationNumInputs(op);
		int num_outputs = TF_OperationNumOutputs(op);
		printf("%s %s %dx%d\n", name, type, num_inputs, num_outputs);
	}
#endif
	tf_brain->input.oper = TF_GraphOperationByName(tf_brain->graph, "Placeholder");
	tf_brain->input.index = 0;
	tf_brain->output.oper = TF_GraphOperationByName(tf_brain->graph, "pi/add");
	tf_brain->output.index = 0;
	
	//tf_brain->output_tensor = TF_AllocateTensor(TF_FLOAT, (int64_t[]) { AI_ACTION_LENGTH }, 1, AI_ACTION_LENGTH * sizeof(float));

	//TF_Operation *init_op = TF_GraphOperationByName(tf_brain->graph, "init");
	//TF_SessionRun(tf_brain->session, NULL,
	//	/* No inputs */
	//	NULL, NULL, 0,
	//	/* No outputs */
	//	NULL, NULL, 0,
	//	/* Just the init operation */
	//	(TF_Operation*[]){init_op}, 1,
	//	/* No metadata */
	//	NULL, tf_brain->status);
	//if (TF_GetCode(tf_brain->status) != TF_OK) {
	//	const char *msg = TF_Message(tf_brain->status);
	//	puts(msg);
	//}
}
void tf_brain_delete(tf_brain_t *tf_brain) {
	//TF_DeleteTensor(tf_brain->output_tensor);
	TF_DeleteTensor(tf_brain->input_tensor);
	TF_DeleteSession(tf_brain->session, tf_brain->status);
	tf_brain_check_status(tf_brain);
	TF_DeleteGraph(tf_brain->graph);
	TF_DeleteStatus(tf_brain->status);
}

int tf_brain_check_status(const tf_brain_t *tf_brain) {
	if (TF_GetCode(tf_brain->status) != TF_OK) {
#if _DEBUG
		const char *msg = TF_Message(tf_brain->status);
		puts(msg);
#endif
		assert(0);
		return 0;
	}
	return 1;
}