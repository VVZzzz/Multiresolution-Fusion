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
	int inital_width;    //ԭͼ�Ŀ��
	int inital_height;   //ԭͼ�ĳ���
	int final_imgsize;   //190213���ؽ�����ͼ�ĳߴ�
	int Rec_min_grid_size;
	int Rec_template_size;
	int beishu;          //190113�����ؽ��ĳߴ���ѵ��ͼ��֮��ı���
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
	int size_number;                                 //ģ���С��Ӧ��������ֵ
	map <int,int> TI_code_value;                     //���ѵ��ͼ��Ķ���ܶȺ���
	map<int,int> Exchange_code_value_white;          //�仯�ı���ֵ
	map<int,int> Exchange_code_value_black;          //�仯�ı���ֵ	
	map <int,int> Reconstruct_code_value;            //����ؽ�ͼ��Ķ���ܶȺ���
	vector<vector<vector<int>>> inital_rand_data;    //��ʼ��С���������ֵ
	vector<vector<vector<int>>> reconstruct_vector;  //����ؽ��������
	vector<vector<vector<int>>> Pre_process_vector;  //���Ԥ��������������ע3*3ȫΪ�ڻ���ȫ�׵����� 
	vector<vector<int>> Inital_image_vector;         //���ԭͼ����  
	vector<int> white_point_x;                       //�������λ��
	vector<int> white_point_y;                       //�������λ��
	vector<int> white_point_z;                       //�������λ��
	vector<int> black_point_x;                       //�������λ��
	vector<int> black_point_y;                       //�������λ��
	vector<int> black_point_z;                       //�������λ��
	int white_rand_site;                             //��ɫ��������λ��
	int black_rand_site;                             //��ɫ��������λ��
	int vector_size;  
	int new_vector_size;   //190113
	vector<vector<vector<int>>> big_3Dimg;       //190211���ͷֱ��ʴ����ά�ṹ

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
