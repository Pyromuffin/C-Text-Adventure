//
// Created by Kelly MacNeill on 5/3/17.
//

#pragma once
#include "utility.h"

typedef uint IndexType;

struct DynamicIndexArray
{
    IndexType* handles;
    uint capacity;
    uint length;
    const char* name;
};

#define MAKE_STATIC_VECTOR(name, capacity)\
static IndexType name##Storage[capacity];\
static DynamicIndexArray name = {&name##Storage[0], capacity, 0 };

#define ITERATE_VECTOR(iteratorName, vector, array) \
for(decltype(&array[0]) iteratorName = (vector->length) ? &array[vector->handles[0]] : NULL, iteratorName##_i = &array[0]; \
(iteratorName##_i - (&array[0])) < vector->length; \
(iteratorName##_i = iteratorName##_i + 1), (iteratorName = &array[vector->handles[(iteratorName##_i - (&array[0]))]] ) )

DynamicIndexArray* AllocateIndexVector(uint capacity, const char* name);
void FreeIndexVector(DynamicIndexArray* vector);

void RemoveSwapBack(DynamicIndexArray *me, IndexType index);
void PushIndex(DynamicIndexArray *me, IndexType index);
void PushIndexStatic(DynamicIndexArray *me, IndexType handle);

IndexType PopIndex(DynamicIndexArray *me);

void ResizeVector( DynamicIndexArray* me, uint newCapacity ); // still doesn't actually shrink.

void InitVectorTracking();
void CheckForVectorLeaks();

// does not preserve ordering.
void DeduplicateIndices(DynamicIndexArray * me); 
void ResetVetor(DynamicIndexArray * me);

