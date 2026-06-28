#include "logger.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#else
#include <sys/stat.h>
#endif

static FILE* log_file = NULL;
static PhaseType current_phase;
static int current_phase_step_count = 0;
static int written_phase_count = 0;
const char* folderPath = "Logs";
const char* filename = "./Logs/compiler_logs.json";

static void write_json_string(FILE* file, const char* value) {
    const unsigned char* cursor = (const unsigned char*)(value ? value : "");
    fputc('"', file);

    while (*cursor) {
        switch (*cursor) {
            case '\\':
                fputs("\\\\", file);
                break;
            case '"':
                fputs("\\\"", file);
                break;
            case '\n':
                fputs("\\n", file);
                break;
            case '\r':
                fputs("\\r", file);
                break;
            case '\t':
                fputs("\\t", file);
                break;
            default:
                if (*cursor < 0x20) {
                    fprintf(file, "\\u%04x", *cursor);
                } else {
                    fputc(*cursor, file);
                }
                break;
        }
        cursor++;
    }

    fputc('"', file);
}

void init_logger() {
    current_phase_step_count = 0;
    written_phase_count = 0;
    #ifdef _WIN32
    if (_mkdir(folderPath) == -1) {
    #else
    if (mkdir(folderPath, 0777) == -1) {
    #endif
    
        if (errno != EEXIST) {
            perror("Error creating directory");
            exit(1);
        }
    }
    log_file = fopen(filename, "w");
    if (!log_file) {
        perror("Could not open log file");
        exit(1);
    }
    fprintf(log_file, "{ \"phases\": [\n"); // JSON start
}

void start_phase(PhaseType phase) {
    current_phase = phase;
    current_phase_step_count = 0;
    const char* phase_name =
        (phase == PHASE_LEX_PARSE) ? "PHASE_LEX_PARSE" :
        (phase == PHASE_SEMANTIC) ? "PHASE_SEMANTIC" :
        "PHASE_ICG";

    fprintf(log_file, "  { \"phase\": \"%s\", \"steps\": [\n", phase_name);
}

void log_step(Step step) {
    if (!log_file) return;

    switch (step.type) {
        case LEX_READ_TOKEN:
            fprintf(log_file, "    { \"type\": \"LEX_READ_TOKEN\", \"data\": {\"token\": ");
            write_json_string(log_file, step.readToken.tokenName);
            fprintf(log_file, ", \"value\": ");
            write_json_string(log_file, step.readToken.value);
            fprintf(log_file, ", \"location\": \"%d:%d\" }},\n", step.readToken.line_no, step.readToken.char_no);
            break;

        case PARSE_REDUCE_RULE:
            fprintf(log_file, "    { \"type\": \"PARSE_REDUCE_RULE\", \"data\": {\"ruleNo\": \"%d\", \"rule\": ", step.reduceRule.ruleNo);
            write_json_string(log_file, step.reduceRule.rule);
            fprintf(log_file, " }},\n");
            break;

        case PARSE_REDUCE_RULE_COMPLETE:
            fprintf(log_file, "    { \"type\": \"PARSE_REDUCE_RULE_COMPLETE\", \"data\": {\"ruleNo\": \"%d\", \"lhs\": ", step.ReduceRuleComplete.ruleNo);
            write_json_string(log_file, step.ReduceRuleComplete.lhs);
            fprintf(log_file, ", \"rhsLength\": \"%d\" }},\n", step.ReduceRuleComplete.rhsLength);
            break; 

        case PARSE_SEMANTIC_STEP:
            fprintf(log_file, "    { \"type\": \"PARSE_SEMANTIC_STEP\", \"data\": {\"ruleNo\": \"%d\", \"stepNo\": \"%d\", \"instr\": ", step.SemanticStep.ruleNo, step.SemanticStep.stepNo);
            write_json_string(log_file, step.SemanticStep.instr);
            fprintf(log_file, " }},\n");
            break;

        case PARSE_CREATE_SCOPE:
            fprintf(log_file, "    { \"type\": \"PARSE_CREATE_SCOPE\", \"data\": {\"table_id\": \"%d\", \"name\": ", step.CreateScope.table_id);
            write_json_string(log_file, step.CreateScope.scopeName);
            fprintf(log_file, ", \"parent_id\": \"%d\" }},\n", step.CreateScope.parent_id);
            break;

        case PARSE_ENTER_SCOPE:
            fprintf(log_file, "    { \"type\": \"PARSE_ENTER_SCOPE\", \"data\": {\"table_id\": \"%d\", \"name\": ", step.EnterScope.table_id);
            write_json_string(log_file, step.EnterScope.scopeName);
            fprintf(log_file, " }},\n");
            break;
        
        case PARSE_EXIT_SCOPE:
            fprintf(log_file, "    { \"type\": \"PARSE_EXIT_SCOPE\", \"data\": {\"table_id\": \"%d\", \"name\": ", step.EnterScope.table_id);
            write_json_string(log_file, step.EnterScope.scopeName);
            fprintf(log_file, " }},\n");
            break; 

        case PARSE_ADD_SYM:
            fprintf(log_file, "    { \"type\": \"PARSE_ADD_SYM\", \"data\": {\"name\": ");
            write_json_string(log_file, step.AddSymbol.name);
            fprintf(log_file, ", \"sym_type\": ");
            write_json_string(log_file, step.AddSymbol.type);
            fprintf(log_file, ", \"scope_id\": \"%d\", \"is_function\": \"%d\", \"line_no\": \"%d\", \"char_no\": \"%d\", \"is_duplicate\": \"%d\" }},\n",
                step.AddSymbol.scope_id,
                step.AddSymbol.is_function,
                step.AddSymbol.line_no,
                step.AddSymbol.char_no,
                step.AddSymbol.is_duplicate);
            break;
        case PARSE_ASSGN_SYM_TYPE:
            fprintf(log_file, "    { \"type\": \"PARSE_ASSGN_SYM_TYPE\", \"data\": {\"name\": ");
            write_json_string(log_file, step.AssignSymType.name);
            fprintf(log_file, ", \"sym_type\": ");
            write_json_string(log_file, step.AssignSymType.type);
            fprintf(log_file, ", \"scope_id\": \"%d\"}},\n", step.AssignSymType.scope_id);
            break;
        case PARSE_CREATE_AST_NODE:
            fprintf(
                log_file,
                "    { \"type\": \"PARSE_CREATE_AST_NODE\", \"data\": {\"node_id\": \"%d\"}},\n",
                step.CreateASTNode.node_id
            ); 
            break;
        case PARSE_ENTERING_STATE: 
            fprintf(
                log_file,
                "    { \"type\": \"PARSE_ENTERING_STATE\", \"data\": {\"state\": \"%d\"}},\n",
                step.ParseEnteringState.state
            );
            break;
        case PARSE_STACK_SNAPSHOT: {
            fprintf(
                log_file,
                "    { \"type\": \"PARSE_STACK_SNAPSHOT\", \"data\": { \"states\": ["
            );

            for (int i = 0; i < step.ParseStackSnapshot.size; i++) {
                fprintf(
                    log_file,
                    "%d%s",
                    step.ParseStackSnapshot.states[i],
                    (i + 1 < step.ParseStackSnapshot.size) ? ", " : ""
                );
            }

            fprintf(
                log_file,
                "], \"size\": \"%d\" }},\n",
                step.ParseStackSnapshot.size
            );
            
            break;
        }

        case PARSE_ERROR:
            fprintf(log_file, "    { \"type\": \"PARSE_ERROR\", \"data\": {\"message\": ");
            write_json_string(log_file, step.ParseError.message);
            fprintf(log_file, ", \"line_no\": \"%d\", \"char_no\": \"%d\" }},\n",
                step.ParseError.line_no,
                step.ParseError.char_no);
            break;

        case SEMANTIC_PASS_STATUS:
            fprintf(log_file, "    { \"type\": \"SEMANTIC_PASS_STATUS\", \"data\": {\"pass\": ");
            write_json_string(log_file, step.SemanticPassStatus.pass);
            fprintf(log_file, ", \"status\": ");
            write_json_string(log_file, step.SemanticPassStatus.status);
            fprintf(log_file, ", \"message\": ");
            write_json_string(log_file, step.SemanticPassStatus.message);
            fprintf(log_file, " }},\n");
            break;

        case SEMANTIC_SYMBOL_HIGHLIGHT:
            fprintf(log_file, "    { \"type\": \"SEMANTIC_SYMBOL_HIGHLIGHT\", \"data\": {\"scope_id\": \"%d\", \"symbol_name\": ", step.SemanticSymbolHighlight.scope_id);
            write_json_string(log_file, step.SemanticSymbolHighlight.symbol_name);
            fprintf(log_file, ", \"reason\": ");
            write_json_string(log_file, step.SemanticSymbolHighlight.reason);
            if (step.SemanticSymbolHighlight.old_type) {
                fprintf(log_file, ", \"old_type\": ");
                write_json_string(log_file, step.SemanticSymbolHighlight.old_type);
            }
            if (step.SemanticSymbolHighlight.new_type) {
                fprintf(log_file, ", \"new_type\": ");
                write_json_string(log_file, step.SemanticSymbolHighlight.new_type);
            }
            if (step.SemanticSymbolHighlight.has_location) {
                fprintf(log_file, ", \"line_no\": \"%d\", \"char_no\": \"%d\"", step.SemanticSymbolHighlight.line_no, step.SemanticSymbolHighlight.char_no);
            }
            fprintf(log_file, " }},\n");
            break;

        case SEMANTIC_NODE_HIGHLIGHT:
            fprintf(log_file, "    { \"type\": \"SEMANTIC_NODE_HIGHLIGHT\", \"data\": {\"pass\": ");
            write_json_string(log_file, step.SemanticNodeHighlight.pass);
            fprintf(log_file, ", \"node_id\": \"%d\", \"node_type\": ", step.SemanticNodeHighlight.node_id);
            write_json_string(log_file, step.SemanticNodeHighlight.node_type);
            fprintf(log_file, ", \"line_no\": \"%d\", \"char_no\": \"%d\", \"action\": ", step.SemanticNodeHighlight.line_no, step.SemanticNodeHighlight.char_no);
            write_json_string(log_file, step.SemanticNodeHighlight.action);
            if (step.SemanticNodeHighlight.message) {
                fprintf(log_file, ", \"message\": ");
                write_json_string(log_file, step.SemanticNodeHighlight.message);
            }
            fprintf(log_file, " }},\n");
            break;

        case SEMANTIC_ERROR_LOG:
            fprintf(log_file, "    { \"type\": \"SEMANTIC_ERROR\", \"data\": {\"pass\": ");
            write_json_string(log_file, step.SemanticErrorLog.pass);
            fprintf(log_file, ", \"message\": ");
            write_json_string(log_file, step.SemanticErrorLog.message);
            fprintf(log_file, ", \"line_no\": \"%d\", \"char_no\": \"%d\"", step.SemanticErrorLog.line_no, step.SemanticErrorLog.char_no);
            if (step.SemanticErrorLog.node_id) {
                fprintf(log_file, ", \"node_id\": ");
                write_json_string(log_file, step.SemanticErrorLog.node_id);
            }
            if (step.SemanticErrorLog.scope_id) {
                fprintf(log_file, ", \"scope_id\": ");
                write_json_string(log_file, step.SemanticErrorLog.scope_id);
            }
            if (step.SemanticErrorLog.symbol_name) {
                fprintf(log_file, ", \"symbol_name\": ");
                write_json_string(log_file, step.SemanticErrorLog.symbol_name);
            }
            fprintf(log_file, " }},\n");
            break;

        case SEMANTIC_ANALYSIS_COMPLETE:
            fprintf(log_file, "    { \"type\": \"SEMANTIC_ANALYSIS_COMPLETE\", \"data\": {\"status\": ");
            write_json_string(log_file, step.SemanticAnalysisComplete.status);
            fprintf(log_file, ", \"total_errors\": \"%d\" }},\n", step.SemanticAnalysisComplete.total_errors);
            break;

        case ICG_NODE_VISIT:
            fprintf(log_file, "    { \"type\": \"ICG_NODE_VISIT\", \"data\": {\"ast_node_id\": \"%d\", \"node_type\": ",
                step.ICGNodeVisit.ast_node_id);
            write_json_string(log_file, step.ICGNodeVisit.node_type);
            fprintf(log_file, ", \"line_no\": \"%d\", \"char_no\": \"%d\", \"action\": ",
                step.ICGNodeVisit.line_no, step.ICGNodeVisit.char_no);
            write_json_string(log_file, step.ICGNodeVisit.action);
            if (step.ICGNodeVisit.operator_name) {
                fprintf(log_file, ", \"operator\": ");
                write_json_string(log_file, step.ICGNodeVisit.operator_name);
            }
            fprintf(log_file, " }},\n");
            break;

        case ICG_CREATE_TEMP:
            fprintf(log_file, "    { \"type\": \"ICG_CREATE_TEMP\", \"data\": {\"ast_node_id\": \"%d\", \"node_type\": ",
                step.ICGCreateTemp.ast_node_id);
            write_json_string(log_file, step.ICGCreateTemp.node_type);
            fprintf(log_file, ", \"line_no\": \"%d\", \"char_no\": \"%d\", \"temp_name\": ",
                step.ICGCreateTemp.line_no, step.ICGCreateTemp.char_no);
            write_json_string(log_file, step.ICGCreateTemp.temp_name);
            fprintf(log_file, " }},\n");
            break;

        case ICG_CREATE_LABEL:
            fprintf(log_file, "    { \"type\": \"ICG_CREATE_LABEL\", \"data\": {\"ast_node_id\": \"%d\", \"node_type\": ",
                step.ICGCreateLabel.ast_node_id);
            write_json_string(log_file, step.ICGCreateLabel.node_type);
            fprintf(log_file, ", \"line_no\": \"%d\", \"char_no\": \"%d\", \"label_name\": ",
                step.ICGCreateLabel.line_no, step.ICGCreateLabel.char_no);
            write_json_string(log_file, step.ICGCreateLabel.label_name);
            fprintf(log_file, ", \"target_tac_id\": \"%d\" }},\n", step.ICGCreateLabel.target_tac_id);
            break;

        case ICG_EMIT:
            fprintf(log_file, "    { \"type\": \"ICG_EMIT\", \"data\": {\"ast_node_id\": \"%d\", \"node_type\": ",
                step.ICGEmit.ast_node_id);
            write_json_string(log_file, step.ICGEmit.node_type);
            fprintf(log_file, ", \"line_no\": \"%d\", \"char_no\": \"%d\", \"instruction_no\": \"%d\", \"source_tac_id\": \"%d\", \"opcode\": ",
                step.ICGEmit.line_no, step.ICGEmit.char_no, step.ICGEmit.instruction_no, step.ICGEmit.source_tac_id);
            write_json_string(log_file, step.ICGEmit.opcode);
            fprintf(log_file, ", \"result\": ");
            write_json_string(log_file, step.ICGEmit.result);
            fprintf(log_file, ", \"arg1\": ");
            write_json_string(log_file, step.ICGEmit.arg1);
            fprintf(log_file, ", \"arg2\": ");
            write_json_string(log_file, step.ICGEmit.arg2);
            fprintf(log_file, ", \"target_label\": ");
            write_json_string(log_file, step.ICGEmit.target_label);
            fprintf(log_file, ", \"text\": ");
            write_json_string(log_file, step.ICGEmit.text);
            fprintf(log_file, " }},\n");
            break;

        case ICG_PATCH_LABEL:
            fprintf(log_file, "    { \"type\": \"ICG_PATCH_LABEL\", \"data\": {\"ast_node_id\": \"%d\", \"node_type\": ",
                step.ICGPatchLabel.ast_node_id);
            write_json_string(log_file, step.ICGPatchLabel.node_type);
            fprintf(log_file, ", \"line_no\": \"%d\", \"char_no\": \"%d\", \"instruction_no\": \"%d\", \"label_name\": ",
                step.ICGPatchLabel.line_no, step.ICGPatchLabel.char_no, step.ICGPatchLabel.instruction_no);
            write_json_string(log_file, step.ICGPatchLabel.label_name);
            fprintf(log_file, ", \"text\": ");
            write_json_string(log_file, step.ICGPatchLabel.text);
            fprintf(log_file, " }},\n");
            break;

        case ICG_ENTER_FUNCTION:
        case ICG_EXIT_FUNCTION:
            fprintf(log_file, "    { \"type\": \"%s\", \"data\": {\"ast_node_id\": \"%d\", \"node_type\": ",
                step.type == ICG_ENTER_FUNCTION ? "ICG_ENTER_FUNCTION" : "ICG_EXIT_FUNCTION",
                step.ICGFunctionEvent.ast_node_id);
            write_json_string(log_file, step.ICGFunctionEvent.node_type);
            fprintf(log_file, ", \"line_no\": \"%d\", \"char_no\": \"%d\", \"function_name\": ",
                step.ICGFunctionEvent.line_no, step.ICGFunctionEvent.char_no);
            write_json_string(log_file, step.ICGFunctionEvent.function_name);
            fprintf(log_file, " }},\n");
            break;

        case ICG_COMPLETE:
            fprintf(log_file, "    { \"type\": \"ICG_COMPLETE\", \"data\": {\"status\": ");
            write_json_string(log_file, step.ICGComplete.status);
            fprintf(log_file, ", \"instruction_count\": \"%d\", \"temporary_count\": \"%d\", \"label_count\": \"%d\" }},\n",
                step.ICGComplete.instruction_count,
                step.ICGComplete.temporary_count,
                step.ICGComplete.label_count);
            break;

        default:
            fprintf(log_file, "    { \"type\": \"UNKNOWN\" },\n");
            break;
    }

    current_phase_step_count++;
}


void end_phase() {
    if (current_phase_step_count > 0) {
        fseek(log_file, -2, SEEK_CUR); // remove last comma
    }
    fprintf(log_file, "\n  ]},\n");
    written_phase_count++;
}

void close_logger() {
    if (written_phase_count > 0) {
        fseek(log_file, -2, SEEK_CUR); // remove last comma
    }
    fprintf(log_file, "\n]}\n");   // JSON end
    fclose(log_file);
}
