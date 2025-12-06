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
const char* folderPath = "Logs";
const char* filename = "./Logs/compiler_logs.json";

void init_logger() {
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
            fprintf(
                log_file,
                "    { \"type\": \"LEX_READ_TOKEN\", \"data\": {\"token\": \"%s\", \"value\": \"%s\", \"location\": \"%d:%d\" }},\n",
                step.readToken.tokenName,
                step.readToken.value,
                step.readToken.line_no,
                step.readToken.char_no
            );
            break;

        case PARSE_REDUCE_RULE:
            fprintf(
                log_file,
                "    { \"type\": \"PARSE_REDUCE_RULE\", \"data\": {\"ruleId\": \"%d\", \"subRuleId\": \"%d\", \"rule\": \"%s\" }},\n",
                step.reduceRule.ruleId,
                step.reduceRule.subRuleId,
                step.reduceRule.rule
            );
            break;

        case PARSE_CREATE_SCOPE:
            fprintf(
                log_file,
                "    { \"type\": \"PARSE_CREATE_SCOPE\", \"data\": {\"table_id\": \"%d\", \"name\": \"%s\", \"parent_id\": \"%d\" }},\n",
                step.CreateScope.table_id,
                step.CreateScope.scopeName,
                step.CreateScope.parent_id
            );
            break;

        case PARSE_ENTER_SCOPE:
            fprintf(
                log_file,
                "    { \"type\": \"PARSE_ENTER_SCOPE\", \"data\": {\"table_id\": \"%d\", \"name\": \"%s\" }},\n",
                step.EnterScope.table_id,
                step.EnterScope.scopeName
            );
            break;
        
        case PARSE_EXIT_SCOPE:
            fprintf(
                log_file,
                "    { \"type\": \"PARSE_EXIT_SCOPE\", \"data\": {\"table_id\": \"%d\", \"name\": \"%s\" }},\n",
                step.EnterScope.table_id,
                step.EnterScope.scopeName
            );
            break; 

        case PARSE_ADD_SYM:
            fprintf(
                log_file,
                "    { \"type\": \"PARSE_ADD_SYM\", \"data\": {\"name\": \"%s\", \"sym_type\": \"%s\", "
                "\"scope_id\": \"%d\", \"is_function\": \"%d\", \"line_no\": \"%d\", \"char_no\": \"%d\", "
                "\"is_duplicate\": \"%d\" }},\n",
                step.AddSymbol.name,
                step.AddSymbol.type,
                step.AddSymbol.scope_id,
                step.AddSymbol.is_function,
                step.AddSymbol.line_no,
                step.AddSymbol.char_no,
                step.AddSymbol.is_duplicate
            );
            break;
        case PARSE_ASSGN_SYM_TYPE:
            fprintf(
                log_file,
                "    { \"type\": \"PARSE_ASSGN_SYM_TYPE\", \"data\": {\"name\": \"%s\", \"sym_type\": \"%s\", "
                "\"scope_id\": \"%d\"}},\n",
                step.AssignSymType.name,
                step.AssignSymType.type,
                step.AssignSymType.scope_id
            );
            break;

        default:
            fprintf(log_file, "    { \"type\": \"UNKNOWN\" },\n");
            break;
    }
}


void end_phase() {
    fseek(log_file, -2, SEEK_CUR); // remove last comma
    fprintf(log_file, "\n  ]},\n");
}

void close_logger() {
    fseek(log_file, -2, SEEK_CUR); // remove last comma
    fprintf(log_file, "\n]}\n");   // JSON end
    fclose(log_file);
}
