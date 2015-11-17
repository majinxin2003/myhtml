//
//  myhtml_parser.c
//  myhtml
//
//  Created by Alexander Borisov on 17.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_parser.h"


void myhtml_parser_index(myhtml_tree_t* tree, myhtml_queue_node_index_t queue_idx, myhtml_token_index_t token_idx)
{
    myhtml_t* myhtml = tree->myhtml;
    mytags_t* mytags = myhtml->tags;
    myhtml_tree_indexes_t* indexes = tree->indexes;
    
    myhtml_token_node_t* token = &tree->token->nodes[token_idx];
    mytags_index_tag_add(mytags, indexes->tags, token->tag_ctx_idx, token_idx);
}

void myhtml_parser_stream(myhtml_tree_t* tree, myhtml_queue_node_index_t queue_idx, myhtml_token_index_t token_idx)
{
    //tree->myhtml->insertion_func[tree->insert_mode](tree, qnode_idx);
    //myhtml_token_print_by_idx(tree, token_idx, stdout);
}

void myhtml_parser_worker(myhtml_tree_t* tree, myhtml_queue_node_index_t queue_idx, myhtml_token_index_t token_idx)
{
    myhtml_t* myhtml                = tree->myhtml;
    myhtml_token_node_t* token      = &tree->token->nodes[token_idx];
    myhtml_queue_t* queue           = myhtml->queue;
    myhtml_queue_node_t* queue_node = &queue->nodes[queue_idx];
    
    if(token->tag_ctx_idx == MyTAGS_TAG__TEXT ||
       token->tag_ctx_idx == MyTAGS_TAG__COMMENT)
    {
        if(token->entry.data == NULL) {
            myhtml_string_init(&token->entry, (queue_node->length + 512));
        }
        
        myhtml_string_t* string = &token->entry;
        
        token->begin      = myhtml_string_len(string);
        token->length     = queue_node->length;
        token->attr_first = 0;
        token->attr_last  = 0;
        
        myhtml_string_append_with_null(string,
                                       &queue_node->html[queue_node->begin],
                                       queue_node->length);
    }
    else if(token->attr_first)
    {
        if(token->entry.data == NULL) {
            myhtml_string_init(&token->entry, 512);
        }
        
        token->begin  = 0;
        token->length = 0;
        
        size_t attr_id = token->attr_first;
        
        while(attr_id)
        {
            myhtml_string_t* string = &token->entry;
            myhtml_token_attr_t* attr = &myhtml_token_attr(tree->token, attr_id);
            
            if(attr->name_length)
            {
                size_t begin = attr->name_begin;
                attr->name_begin = myhtml_string_len(string);
                
                myhtml_string_append_lowercase_with_null(string,
                                                         &queue_node->html[begin],
                                                         attr->name_length);
            }
            
            if(attr->value_length)
            {
                size_t begin = attr->value_begin;
                attr->value_begin = myhtml_string_len(string);
                
                myhtml_string_append_with_null(string,
                                               &queue_node->html[begin],
                                               attr->value_length);
            }
            
            attr_id = attr->next;
        }
    }
    
    pthread_mutex_lock(&tree->myhtml->thread->global_mutex);
    myhtml_token_print_by_idx(tree, token_idx, stdout);
    pthread_mutex_unlock(&tree->myhtml->thread->global_mutex);
}


