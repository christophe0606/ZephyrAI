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

struct tfliteNode
{
   uint8_t *modelAddr;
   size_t modelSize;
};

struct GraphaParams
{
    // Name of struct is the name of the node as defined
    // in Python graph.
    struct classifyParams classify;
    struct tfliteNode kws;
};

extern struct GraphaParams graphaParams;

#ifdef   __cplusplus
}
#endif

#endif