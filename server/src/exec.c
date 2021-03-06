#include <malloc.h>
#include <string.h>
#include <stdarg.h>

#include "database.h"
#include "strptime.h"
#include "analyze.h"
#include "select.h"
#include "shell.h"
#include "debug.h"
#include "ast.h"

char single_command[COMMAND_BUFFER_LENGTH];
ExprNode error_expr = {.type = EXPR_ERROR},
         null_expr = {.type = EXPR_NULL},
         lazy_expr = {.type = EXPR_LAZY},
         zero_expr = {.type = EXPR_INTNUM, .intval = 0},
         one_expr = {.type = EXPR_INTNUM, .intval = 1};
byte query_status;
clock_t op_start, op_end;

inline int write_message (char *s, ...)
{
    va_list ap;
    va_start (ap, s);
    switch (crims_status)
    {
    case STATUS_SHELL:
        vfprintf (stderr, s, ap);
        fprintf (stderr, "\n");
        break;
    }
}

/*
    执行单句SQL
*/
inline int exec_single (char *sql)
{
    op_start = clock();
    query_initialize();
    SqlAst *root = parse_sql (sql);
    //print_ast(root, 0);
    if (root == NULL)
    {
        return STATUS_ERROR;
    }
    else if (root->type == SELECT_STMT)
    {
        // if (check_select (root->select, NULL))
        // {
        //     return STATUS_ERROR;
        // }
        byte res = do_select (root->select, &rec, &recs, 0,
                              is_grpby = (root->select->group != NULL),
                              is_odrby = (root->select->order != NULL));
        if (res != ERROR && crims_status == STATUS_SHELL)
        {
            print_result (&recs);
        }
        return res;
    }
    else if (root->type == DELETE_STMT)
    {
        // if (check_select (root->select, NULL))
        // {
        //     return STATUS_ERROR;
        // }
        return 0;
    }
    else if (root->type == INSERT_STMT)
    {
        // if (check_select (root->select, NULL))
        // {
        //     return STATUS_ERROR;
        // }
        return 0;
    }
    else if (root->type == UPDATE_STMT)
    {
        // if (check_select (root->select, NULL))
        // {
        //     return STATUS_ERROR;
        // }
        return 0;
    }
    else
    {
        return STATUS_UNKNOWN;
    }
}

inline int exec (char *command)
{
    int l = strlen (command), start = 0;
    int res = 0;
    for (int i = 0; i < l; ++i)
    {
        if (command[i] == ';')
        {
            strncpy (single_command, command + start, i - start + 1);
            start = i + 1;
            single_command[start] = '\0';
            res = exec_single (single_command);
        }
    }
    return res;
}