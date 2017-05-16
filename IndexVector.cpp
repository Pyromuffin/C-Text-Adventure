//
// Created by Kelly MacNeill on 5/3/17.
//

#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include "IndexVector.h"

static const uint MAX_VECTORS = 100;

static DynamicIndexArray s_AllVectors[MAX_VECTORS]; // this is only for tracking purposes, to make sure we're not leaking.
MAKE_STATIC_VECTOR(s_FreeList, MAX_VECTORS);

void PushIndexStatic(DynamicIndexArray *me, IndexType handle) {
    assert(me->length <  me->capacity);
    assert(me->capacity > 0);
    me->handles[me->length] = handle;
    me->length++;
}

void ResizeVector( DynamicIndexArray* me, uint newCapacity )
{
    //printf("Resizing vector from %d to %d, length %d\n", me->capacity, newCapacity, me->length);
    //@todo perhaps consider the ability to get smaller.
    assert(newCapacity > me->capacity);
    IndexType* newStorage = (IndexType*) malloc(sizeof(IndexType) * newCapacity);

    //I sure hope me is right??????
    if((me->capacity > 0) && (me->handles != NULL))
    {
        memcpy(newStorage, me->handles, me->length * sizeof(IndexType));
        free(me->handles);
    }
	me->capacity = newCapacity;
	me->handles = newStorage;
}

void PushIndex(DynamicIndexArray *me, IndexType index)
{
    if(me->capacity <= me->length )
    {
        ResizeVector(me, me->capacity*2);
    }

	me->handles[me->length] = index;
	me->length++;
}

IndexType PopIndex(DynamicIndexArray *me)
{
    assert(me->length > 0);
    assert(me->capacity > 0);
    me->length--;
    return me->handles[me->length];
}

DynamicIndexArray* AllocateIndexVector(uint capacity, char* name)
{
    IndexType freeVector = PopIndex(&s_FreeList);
    DynamicIndexArray* vector = &s_AllVectors[freeVector];
    vector->name = name;
    assert(vector->capacity == 0);
    ResizeVector(vector, capacity);

    //printf("alloc index %d\n", freeVector);

    return vector;
}

void FreeIndexVector(DynamicIndexArray* vector)
{
    assert(vector->handles);
    assert(vector->capacity > 0);
    free(vector->handles);
    vector->capacity = 0;
    vector->length = 0;
    vector->handles = NULL;

    IndexType offset = vector - &s_AllVectors[0]; // don't forget about how pointer arithmetic works.
    assert(offset >= 0);
    assert(offset < MAX_VECTORS);

    //printf("freed index %d\n", offset);
    PushIndexStatic( &s_FreeList, offset );
}

void InitVectorTracking()
{
    for(uint i = 0; i < MAX_VECTORS; i++)
    {
        PushIndexStatic(&s_FreeList, (IndexType)i);
    }
}

void CheckForVectorLeaks()
{
    assert(s_FreeList.length == MAX_VECTORS);
    bool foundIndices[MAX_VECTORS];
    memset(foundIndices, 0, sizeof(foundIndices));

    for( int i =0; i < MAX_VECTORS; i ++)
    {
        IndexType index = PopIndex(&s_FreeList);
        if(foundIndices[index])
        {
            printf("Double freed index %d, position %d, vector %s\n", index, i, s_AllVectors[index].name);
        }

        foundIndices[index] = true;
    }

    for( int i =0; i < MAX_VECTORS; i ++)
    {
        if(!foundIndices[i])
        {
            printf("Leaked Index %d, vector %s\n", i, s_AllVectors[i].name);
        }
    }
}
