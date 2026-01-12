#ifndef APPA_PARAMS_H
#define APPA_PARAMS_H

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

struct AppaParams
{
    // Name of struct is the name of the node as defined
    // in Python graph.
    struct classifyParams classify;
    struct tfliteNode kws;
};

extern struct AppaParams appaParams;

#ifdef   __cplusplus
}
#endif

#endif