/*
#pragma once
#include "stdafx.h"
#include <map>
#include <iostream>
#include <highgui.h>
#include <cstdlib>
#include <cxcore.h >
#include <math.h>
using namespace std;
inline unsigned __int32 rand32();
class CAnnealing
{
public:
	CAnnealing(void);
	~CAnnealing(void);
	CAnnealing(IplImage* img, int width, int height,int template_size);
	uchar *image;
	int inital_width;    //原图的宽度
	int inital_height;   //原图的长度
	int final_imgsize;   //190213：重建最终图的尺寸
	int Rec_min_grid_size;
	int Rec_template_size;
	int beishu;          //190113期望重建的尺寸与训练图像之间的倍数
	void Set_inital_data();
	void Transfer_grid_size(int);
	void TI_Multi_point_density_function();
	void TI_Multi_point_density_function_p();
	void Recstruct_Multi_point_density_function();
	void Recstruct_Multi_point_density_function_fast(int&,int&,int&);
	void Recstruct_Multi_point_density_function_fast_p(int&,int&,int&);
	void Recover_Multi_point_density_function();
	void Calculate_E_differ(int&);
	void Exchange_two_point();
	void Recover_exchange();
	void Set_reconstruct_to_inital(); 
	void Set_inital_to_reconstruct_uncondition_1();
	void Set_inital_to_reconstruct_uncondition_2();
	void Set_inital_to_reconstruct_uncondition_3();
	void Putout_step_image();
	void Delete_exchange_point();
	void Delete_unexchange_two_phase_point();
	void Delete_unexchange_white_point();
	void Delete_unexchange_black_point();
	void Reverse_exchange_point();
	void Delete_site_vector();
	void Select_inital_exchange_point_uncondition(int&,int&);
	void Select_final_exchange_point_uncondition(int&,int&);
	void Select_final_exchange_point_uncondition_fast(int&,int&);
	void Select_final_exchange_point_uncondition_test(int&,int&);
	void Decide_min_grid(int&);
	void Transport_min_grid(int);
	void Putout_inital_image();
	void Random_position(int);
	void Get_exchange_site();
	void Recstruct_Multi_point_density_function_p();
private:
	int Enerry_origial;
	int Enerry_Reverse;
	int grid_size;
	int size_number;                                 //模板大小对应编码的最大值
	map <int,int> TI_code_value;                     //存放训练图像的多点密度函数
	map<int,int> Exchange_code_value_white;          //变化的编码值
	map<int,int> Exchange_code_value_black;          //变化的编码值	
	map <int,int> Reconstruct_code_value;            //存放重建图像的多点密度函数
	vector<vector<vector<int>>> inital_rand_data;    //初始最小容器随机赋值
	vector<vector<vector<int>>> reconstruct_vector;  //存放重建结果容器
	vector<vector<vector<int>>> Pre_process_vector;  //存放预处理结果容器，标注3*3全为黑或者全白的区域 
	vector<vector<int>> Inital_image_vector;         //存放原图容器  
	vector<int> white_point_x;                       //交换点的位置
	vector<int> white_point_y;                       //交换点的位置
	vector<int> white_point_z;                       //交换点的位置
	vector<int> black_point_x;                       //交换点的位置
	vector<int> black_point_y;                       //交换点的位置
	vector<int> black_point_z;                       //交换点的位置
	int white_rand_site;                             //白色交换点数位置
	int black_rand_site;                             //黑色交换点数位置
	int vector_size;  
	int new_vector_size;   //190113
	vector<vector<vector<int>>> big_3Dimg;       //190211：低分辨率大孔三维结构

	vector<int>::iterator white_site_x;
	vector<int>::iterator white_site_y;
	vector<int>::iterator white_site_z;
	vector<int>::iterator black_site_x;
	vector<int>::iterator black_site_y; 
	vector<int>::iterator black_site_z;
	int class_white_point_number;
	int class_black_point_number;
public:
	bool get_filelist_from_dir(string path, vector<string>& files);
	void Set_reconstruct_to_final(void);
	void Putout_final_image(void);
};
*/
