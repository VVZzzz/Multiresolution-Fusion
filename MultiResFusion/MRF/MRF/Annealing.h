/**
 * 该头文件是进行三维重建和二维融合三维的头文件
 */
#pragma once
#include <math.h>
#include <QFileInfoList>
#include <QImage>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
inline unsigned __int32 rand32();
class CAnnealing {
 public:
  CAnnealing(void);
  ~CAnnealing(void);
  CAnnealing(const QString& imgpath, int final_imgsz, int template_size);

  // unsigned char* image;  //wrh 这个发现没用
  int m_initial_width;   //原图的宽度
  int m_initial_height;  //原图的长度
  int m_final_imgsize;   // 190213：重建最终图的尺寸
  int m_Rec_min_grid_size;
  int m_Rec_template_size;
  int m_multiple;  // 190113期望重建的尺寸与训练图像之间的倍数
  void Set_inital_data();
  void Transfer_grid_size(int);
  void TI_Multi_point_density_function();
  void TI_Multi_point_density_function_p();
  void Recstruct_Multi_point_density_function();
  void Recstruct_Multi_point_density_function_fast(int&, int&, int&);
  void Recstruct_Multi_point_density_function_fast_p(int&, int&, int&);
  void Recover_Multi_point_density_function();
  void Calculate_E_differ(int&);
  void Exchange_two_point();
  void Recover_exchange();
  void Set_reconstruct_to_inital();
  void Set_inital_to_reconstruct_uncondition_1();
  void Set_inital_to_reconstruct_uncondition_2();
  void Set_inital_to_reconstruct_uncondition_3();
  void Putout_step_image(const QString &);
  void Delete_exchange_point();
  void Delete_unexchange_two_phase_point();
  void Delete_unexchange_white_point();
  void Delete_unexchange_black_point();
  void Reverse_exchange_point();
  void Delete_site_vector();
  void Select_inital_exchange_point_uncondition(int&, int&);
  void Select_final_exchange_point_uncondition(int&, int&);
  void Select_final_exchange_point_uncondition_fast(int&, int&);
  void Select_final_exchange_point_uncondition_test(int&, int&);
  void Decide_min_grid(int&);
  void Transport_min_grid(int);
  void Putout_inital_image(const QString &);
  void Random_position(int);
  void Get_exchange_site();
  void Recstruct_Multi_point_density_function_p();

 private:
  void LoadInitialImg(const QString& imgpath);
  bool Load3DImg(const QFileInfoList& filelist);

 private:
  int m_Enerry_origial;
  int m_Enerry_Reverse;
  int m_gridsize;
  int m_sizenum;                     //模板大小对应编码的最大值
  map<int, int> m_train_code_value;  //存放训练图像的多点密度函数
  map<int, int> m_exchange_code_value_white;  //变化的编码值
  map<int, int> m_exchange_code_value_black;  //变化的编码值
  map<int, int> m_reconstruct_code_value;  //存放重建图像的多点密度函数
  vector<vector<vector<int>>> m_initial_rand_data;  //初始最小容器随机赋值
  vector<vector<vector<int>>> m_reconstruct_vec;  //存放重建结果容器
  vector<vector<vector<int>>>
      m_preproc_vector;  //存放预处理结果容器，标注3*3全为黑或者全白的区域
  vector<vector<int>> m_initial_imgvec;  //存放原图容器
  vector<int> m_white_posx;              //交换点的位置
  vector<int> m_white_posy;              //交换点的位置
  vector<int> m_white_posz;              //交换点的位置
  vector<int> m_black_posx;              //交换点的位置
  vector<int> m_black_posy;              //交换点的位置
  vector<int> m_black_posz;              //交换点的位置
  int m_white_randsite;                  //白色交换点数位置
  int m_black_randsite;                  //黑色交换点数位置
  int m_vector_size;
  int m_new_vector_size;                       // 190113
  vector<vector<vector<int>>> m_big_3Dimgvec;  // 190211：低分辨率大孔三维结构

  vector<int>::iterator m_white_site_x;
  vector<int>::iterator m_white_site_y;
  vector<int>::iterator m_white_site_z;
  vector<int>::iterator m_black_site_x;
  vector<int>::iterator m_black_site_y;
  vector<int>::iterator m_black_site_z;
  int m_class_white_point_number;
  int m_class_black_point_number;

 public:
  //bool get_filelist_from_dir(string path, vector<string>& files);
  void Set_reconstruct_to_final(void);
  void Putout_final_image(const QString &);
};
