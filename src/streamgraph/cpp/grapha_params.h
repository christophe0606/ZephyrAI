#ifndef GRAPHA_PARAMS_H
#define GRAPHA_PARAMS_H

#ifdef   __cplusplus
extern "C"
{
#endif

struct classifyParams
{
    int historyLength;
};

struct GraphaParams
{
    // Name of struct is the name of the node as defined
    // in Python graph.
    struct classifyParams classify;
};

extern struct GraphaParams graphaParams;

#ifdef   __cplusplus
}
#endif

#endif