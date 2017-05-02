//
// Created by Kelly MacNeill on 5/1/17.
//

#pragma once


typedef enum ItemFlags
{
    ItemFlagUsable = 1 << 0,qui

} ItemFlags;




typedef struct Item
{
    char* name;
    char* description;
    ItemFlags flags;

} Item;
