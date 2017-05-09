//
// Created by Kelly MacNeill on 5/3/17.
//

#include <assert.h>
#include <memory.h>
#include <printf.h>
#include "IndexVector.h"

static const uint MAX_VECTORS = 100;

static DynamicIndexArray s_AllVectors[MAX_VECTORS]; // this is only for tracking purposes, to make sure we're not leaking.
MAKE_STATIC_VECTOR(s_FreeList, MAX_VECTORS);

void PushIndexStatic(DynamicIndexArray *this, IndexType handle) {
    assert(this->length <  this->capacity);
    assert(this->capacity > 0);
    this->handles[this->length] = handle;
    this->length++;
}

void ResizeVector( DynamicIndexArray* this, uint newCapacity )
{
    //printf("Resizing vector from %d to %d, length %d\n", this->capacity, newCapacity, this->length);
    //@todo perhaps consider the ability to get smaller.
    assert(newCapacity > this->capacity);
    IndexType* newStorage = (IndexType*) malloc(sizeof(IndexType) * newCapacity);

    //I sure hope this is right??????
    if((this->capacity > 0) && (this->handles != NULL))
    {
        memcpy(newStorage, this->handles, this->length * sizeof(IndexType));
        free(this->handles);
    }
    this->capacity = newCapacity;
    this->handles = newStorage;
}

void PushIndex(DynamicIndexArray *this, IndexType index)
{
    if( this->capacity <= this->length )
    {
        ResizeVector(this, this->capacity*2);
    }

    this->handles[this->length] = index;
    this->length++;
}

IndexType PopIndex(DynamicIndexArray *this)
{
    assert(this->length > 0);
    assert(this->capacity > 0);
    this->length--;
    return this->handles[this->length];
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
    bzero(foundIndices, sizeof(foundIndices));

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
