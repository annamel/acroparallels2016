//
//  best_error_define_ever.h
//  mf_project
//
//  Created by IVAN MATVEEV on 19.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#ifndef best_error_define_ever_h
#define best_error_define_ever_h

#define error_with_malloc(bool,ret_val) do {\
    if (bool){\
        errno = ENOMEM;\
        return (ret_val);\
    }} while(0);

#endif /* best_error_define_ever_h */
