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
}IndexVector;

#define MAKE_STATIC_VECTOR(name, capacity)\
static IndexType name##Storage[capacity];\
static IndexVector name = {&name##Storage[0], capacity, 0 };


IndexVector* AllocateIndexVector(uint capacity, char* name);
void FreeIndexVector(IndexVector* vector);

void PushIndex(IndexVector *this, IndexType index);
void PushIndexStatic(IndexVector *this, IndexType handle);
IndexType PopIndex(IndexVector *this);

void ResizeVector( IndexVector* this, uint newCapacity );

void InitVectorTracking();
void CheckForVectorLeaks();


