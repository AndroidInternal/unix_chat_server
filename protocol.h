// =====================================================================================
// 
//       Filename:  protocol.h
// 
//    Description:  
// 
//        Version:  1.0
//        Created:  02/21/2016 01:20:40 PM
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  李锐, root@lirui.in
//        License:  Copyright © 2016 李锐. All Rights Reserved.
// 
// =====================================================================================
struct Message
{
    int uid;
    char nickname[24];
    char content[1024];
    int t_uid;//t_uid is < 0 means send to all
};
