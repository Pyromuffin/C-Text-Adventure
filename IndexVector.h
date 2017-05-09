//
// Created by Kelly MacNeill on 5/3/17.
//

#pragma once
#include "utility.h"
#include "items.h"

typedef uint IndexType;

typedef struct IndexVector
{
    IndexType* handles;
    uint capacity;
    uint length;
    char* name;
}DynamicIndexArray;

#define MAKE_STATIC_VECTOR(name, capacity)\
static IndexType name##Storage[capacity];\
static DynamicIndexArray name = {&name##Storage[0], capacity, 0 };

#define ITERATE_VECTOR(iteratorName, vector, array) \
for(typeof(&array[0]) iteratorName = (vector->length) ? &array[vector->handles[0]] : NULL, iteratorName##_i = &array[0]; \
(iteratorName##_i - (&array[0])) < vector->length; \
(iteratorName##_i = iteratorName##_i + 1), (iteratorName = &array[vector->handles[(iteratorName##_i - (&array[0]))]] ) )

DynamicIndexArray* AllocateIndexVector(uint capacity, char* name);
void FreeIndexVector(DynamicIndexArray* vector);

void PushIndex(DynamicIndexArray *this, IndexType index);
void PushIndexStatic(DynamicIndexArray *this, IndexType handle);
IndexType PopIndex(DynamicIndexArray *this);

void ResizeVector( DynamicIndexArray* this, uint newCapacity );

void InitVectorTracking();
void CheckForVectorLeaks();


