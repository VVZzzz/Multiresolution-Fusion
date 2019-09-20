#include "Annealing.h"
//#include <atlstr.h>
#include <stdio.h>
#include <QDir>
#include <QMessageBox>
#include <fstream>
#include <vector>
#include "tools.h"
using namespace std;

CAnnealing::CAnnealing(void) {
}

CAnnealing::~CAnnealing(void) {}

CAnnealing::CAnnealing(const QString& imgpath, int final_imgsz,
                       int template_size) : QObject() {
  m_class_white_point_number = 0;
  m_class_black_point_number = 0;
  m_gridsize = 0;
  m_vector_size = 0;
  m_Rec_template_size = template_size;  //析构函数传递采用模板大小参数。
  m_sizenum = (int)pow(2.0, pow(m_Rec_template_size, 2.0));
  m_Enerry_origial = 0;
  ;
  m_Enerry_Reverse = 0;
  m_final_imgsize = final_imgsz;

  //加载图片到m_initial_imgvec
  LoadInitialImg(imgpath);
  m_multiple = m_final_imgsize / m_initial_width;

  //取消操作标志
  m_ShutDown = false;

}

void CAnnealing::SetSavePath(const QString& dstimgpath) {
  m_dstimg_path = dstimgpath;
}

bool CAnnealing::Reconstruct() {
  m_Rec_template_size = 3;  //选择多点密度函数模板的尺寸大小,默认为3
  m_Rec_min_grid_size = 16;  //最小网格时图像的尺寸大小,默认16
  //共需要的扩展级数
  int grid_times =
      (log((float)m_initial_width) - log((float)m_Rec_min_grid_size)) /
      log(2.0);
  //先定义一个三维容器，用于存放初始最小网格
  //(采样得到二维，再根据孔隙度（inital_white_point*(m_multiple*m_multiple)*new_inital_size）给三维赋初值），在m_initial_rand_data中
  //设置最初随机数据与Set_inital_to_reconstruct_uncondition_1配合使用
  // Set_inital_data();
  Set_inital_data_Recons();

  int judgements_times = 0;
  int energy_up = 0;             //交换前的能量大小
  int energy_down = 0;           //交换后的能量大小
  int energy_differ = 0;         //相邻两次的能量差
  int energy_jump = 0;           //对能量差的限制
  int energy_judge = 0;          //外循环跳出条件
  int percent = 0.0005;          //能量差百分比,默认0.0005不必修改
  int adjust_time = 0;           //判断极端情况是否出现
  int refuse_times_out = 0;      //拒绝次数
  int max_refuse_times_out = 1;  //最大拒绝次数
  int loop_in = 0;               //控制内部循环
  int loop_out = 0;              //控制外部循环
  int loop_in_times = 0;         //内部循环次数
  int loop_out_times = 200;      //外部循环次数
  int main_white_point_number = 0;  //选择白色交换点数量
  int main_black_point_number = 0;  //选择黑色交换点数量

  int energy_process = 0;
  int engerry_change_white = 0;
  int engerry_change_black = 0;
  int judgement_times = 0;

  int prgblock = 100 / (grid_times + 1);
  for (int i = 0; i != grid_times + 1; ++i) {
    if (m_ShutDown) return false;
    //发送进度
    int currprg = i * prgblock;
    emit CurrProgress(currprg);
    
    judgement_times = 0;
    Transfer_grid_size(i);  //获取当前网格的图像尺寸给m_new_vector_size
    TI_Multi_point_density_function_p();  //计算对应网格大小为i时图像的多点密度函数
    Putout_inital_image(m_dstimg_path);  //输出每一级原始图像对应网格图像
    if (i == 0) {
      //将随机数据放置到重建容器m_reconstruct_vec[i][j][k]中,针对最开始尺寸
      //大部分工作已经在Set_inital_data中做过了
      Set_inital_to_reconstruct_uncondition_1();
    } else {
      //先将上级网格点中全1和全0位置在当前网格中配置为全1和全0（从第一级三维开始就已经乘过倍数了），其余再配置随机数
      //按照TI中白色点配置随机数
      Set_inital_to_reconstruct_uncondition_2_Recons();
    }
    Recstruct_Multi_point_density_function_p();  //计算对应网格大小为i时重建图像的多点密度函数
    Calculate_E_differ(energy_up);  //计算多点密度函数能量差值
    // cout << "origial_Energy_up:" << Energy_up << endl;   //刚布好点后的energy
    // energy_limit = percent*Energy_up;    // 这个energy_limit没有用到啊 from
    // wrh

    energy_judge = energy_up;
    adjust_time = energy_up;
    energy_jump = 0;

    float inner_prgblock = prgblock * 1.0 / loop_out_times;  //wrh
    for (loop_out = 0; loop_out < loop_out_times; loop_out++) {
      if (m_ShutDown) return false;
      emit CurrProgress(currprg + loop_out * inner_prgblock);  //wrh
      if (i <= (grid_times + 1) / 2)  //设置不使用相邻反相选点法的网络级数
      {
        //所有点都交换，把待交换的点选择好后坐标放入white_point_x等容器中
        Select_inital_exchange_point_uncondition_Recons(main_white_point_number,
                                                 main_black_point_number);
      } else {
        // temperory中放交换的点，也是放入white_point_x等容器中
        Select_final_exchange_point_uncondition_Recons(main_white_point_number,
                                                main_black_point_number);
        energy_process = energy_up;
      }
      Random_position(loop_out);
      //由于是不同相点之间的相位互换，所以应将较少数量的相作为交换的最大次数
      if (main_black_point_number > main_white_point_number) {
        loop_in_times = main_white_point_number;
      } else {
        loop_in_times = main_black_point_number;
      }
      for (loop_in = 0; loop_in != loop_in_times; loop_in++)  //真正的在交换
      {
        if (m_ShutDown) return false;
        if (main_white_point_number != 0 && main_black_point_number != 0) {
          //确定在white_point_x(black)中的偏移位置white_rand_site
          //从备选点集合中随机交换的两点
          Exchange_two_point();
          Get_exchange_site();  //得到交换两点的坐标，为后面使用
                                //快速计算多点密度函数
          Recstruct_Multi_point_density_function_fast_p(
              energy_down, engerry_change_white, engerry_change_black);

          energy_differ = energy_down - energy_up;
          if (energy_differ <= 0) {
            energy_up = energy_down;  //用新的较小的能量作为Energy_up
            Delete_exchange_point();  //将交换后的点删除，并标记出来，
            main_black_point_number--;
            main_white_point_number--;
          } else if (judgement_times >= 1) {
            energy_up = energy_down;
            Delete_exchange_point();  //将交换后的点删除，并标记出来，
            main_black_point_number--;
            main_white_point_number--;
            judgement_times = 0;
          } else {  //说明当前位置的白点不能改变相位（占大头，不能交换），黑点需要改变，但需要与另一个白点进行交换（所以不delete）
            if (engerry_change_white > 0 && engerry_change_black < 0) {
              Recover_Multi_point_density_function();  //恢复到前一状态的多点密度函数
              Reverse_exchange_point();  //将不满足条件的交换点复原，
              Delete_unexchange_white_point();  //将交换后的点删除，
              main_white_point_number--;
            } else if (engerry_change_white < 0 && engerry_change_black > 0) {
              Recover_Multi_point_density_function();  //恢复到前一状态的多点密度函数
              Reverse_exchange_point();  //将不满足条件的交换点复原，
              Delete_unexchange_black_point();  //将交换后的点删除，
              main_black_point_number--;
            } else {
              Recover_Multi_point_density_function();  //恢复到前一状态的多点密度函数
              Reverse_exchange_point();  //将不满足条件的交换点复原，
              Delete_unexchange_two_phase_point();  //将交换后的点删除，
              main_black_point_number--;
              main_white_point_number--;
            }
          }
        }
        if (main_white_point_number == 0 || main_black_point_number == 0) {
          break;
        }
      }
      Delete_site_vector();  //将保持交换点的容器清空，以备下一轮循环，
      if (adjust_time == energy_up) {
        judgement_times++;
      }
      ////////////////////////////////////
      if (energy_jump == energy_up) {
        refuse_times_out++;
      } else {
        energy_jump = energy_up;
        refuse_times_out = 0;
      }
      // cout << "Energy_up:" << Energy_up << endl;
      ////////////////////////////////////
      if (refuse_times_out >= max_refuse_times_out) {
        break;
      }

      // if ((Energy_judge-Energy_up)<Energy_limit && refuse_times_out==0)
      if ((float)(energy_judge - energy_up) / energy_up < 0.005 &&
          refuse_times_out == 0) {
        break;
      } else {
        energy_judge = energy_up;
      }
      /////////////////////////////////////
    }
    Set_reconstruct_to_inital();  //将reconstructed中的中间重建结果转移到inital
    Putout_step_image(m_dstimg_path);  //输出原始图像对应网格图像
  }

  return true;
}

bool CAnnealing::TwoFuseThree() { 
  
  m_Rec_template_size = 3;  //选择多点密度函数模板的尺寸大小,默认为3
  m_Rec_min_grid_size = 16;  //最小网格时图像的尺寸大小,默认16
  //共需要的扩展级数
  int grid_times =
      (log((float)m_initial_width) - log((float)m_Rec_min_grid_size)) /
      log(2.0);
  //先定义一个三维容器，用于存放初始最小网格
  //(采样得到二维，再根据孔隙度（inital_white_point*(m_multiple*m_multiple)*new_inital_size）给三维赋初值），在m_initial_rand_data中
  //设置最初随机数据与Set_inital_to_reconstruct_uncondition_1配合使用
  // Set_inital_data();
  Set_inital_data_2fuse3();

  int judgements_times = 0;
  int energy_up = 0;             //交换前的能量大小
  int energy_down = 0;           //交换后的能量大小
  int energy_differ = 0;         //相邻两次的能量差
  int energy_jump = 0;           //对能量差的限制
  int energy_judge = 0;          //外循环跳出条件
  int percent = 0.0005;          //能量差百分比,默认0.0005不必修改
  int adjust_time = 0;           //判断极端情况是否出现
  int refuse_times_out = 0;      //拒绝次数
  int max_refuse_times_out = 1;  //最大拒绝次数
  int loop_in = 0;               //控制内部循环
  int loop_out = 0;              //控制外部循环
  int loop_in_times = 0;         //内部循环次数
  int loop_out_times = 200;      //外部循环次数
  int main_white_point_number = 0;  //选择白色交换点数量
  int main_black_point_number = 0;  //选择黑色交换点数量

  int energy_process = 0;
  int engerry_change_white = 0;
  int engerry_change_black = 0;
  int judgement_times = 0;

  int prgblock = 100 / (grid_times + 1);
  for (int i = 0; i != grid_times + 1; ++i) {
    if (m_ShutDown) return false;
    //发送进度
    int currprg = i * prgblock;
    emit CurrProgress(currprg);
    
    judgement_times = 0;
    Transfer_grid_size(i);  //获取当前网格的图像尺寸给m_new_vector_size
    TI_Multi_point_density_function_p();  //计算对应网格大小为i时图像的多点密度函数
    Putout_inital_image(m_dstimg_path);  //输出每一级原始图像对应网格图像
    if (i == 0) {
      //将随机数据放置到重建容器m_reconstruct_vec[i][j][k]中,针对最开始尺寸
      //大部分工作已经在Set_inital_data中做过了
      Set_inital_to_reconstruct_uncondition_1();
    } else {
      //先将上级网格点中全1和全0位置在当前网格中配置为全1和全0（从第一级三维开始就已经乘过倍数了），其余再配置随机数
      //按照TI中白色点配置随机数
      Set_inital_to_reconstruct_uncondition_2_2fuse3();
    }
    Recstruct_Multi_point_density_function_p();  //计算对应网格大小为i时重建图像的多点密度函数
    Calculate_E_differ(energy_up);  //计算多点密度函数能量差值
    // cout << "origial_Energy_up:" << Energy_up << endl;   //刚布好点后的energy
    // energy_limit = percent*Energy_up;    // 这个energy_limit没有用到啊 from
    // wrh

    energy_judge = energy_up;
    adjust_time = energy_up;
    energy_jump = 0;

    float inner_prgblock = prgblock * 1.0 / loop_out_times;  //wrh
    for (loop_out = 0; loop_out < loop_out_times; loop_out++) {
      if (m_ShutDown) return false;
      emit CurrProgress(currprg + loop_out * inner_prgblock);  //wrh
      if (i <= (grid_times + 1) / 2)  //设置不使用相邻反相选点法的网络级数
      {
        //所有点都交换，把待交换的点选择好后坐标放入white_point_x等容器中
        Select_inital_exchange_point_uncondition_2fuse3(main_white_point_number,
                                                 main_black_point_number);
      } else {
        // temperory中放交换的点，也是放入white_point_x等容器中
        Select_final_exchange_point_uncondition_2fuse3(main_white_point_number,
                                                main_black_point_number);
        energy_process = energy_up;
      }
      Random_position(loop_out);
      //由于是不同相点之间的相位互换，所以应将较少数量的相作为交换的最大次数
      if (main_black_point_number > main_white_point_number) {
        loop_in_times = main_white_point_number;
      } else {
        loop_in_times = main_black_point_number;
      }
      for (loop_in = 0; loop_in != loop_in_times; loop_in++)  //真正的在交换
      {
        if (m_ShutDown) return false;
        if (main_white_point_number != 0 && main_black_point_number != 0) {
          //确定在white_point_x(black)中的偏移位置white_rand_site
          //从备选点集合中随机交换的两点
          Exchange_two_point();
          Get_exchange_site();  //得到交换两点的坐标，为后面使用
                                //快速计算多点密度函数
          Recstruct_Multi_point_density_function_fast_p(
              energy_down, engerry_change_white, engerry_change_black);

          energy_differ = energy_down - energy_up;
          if (energy_differ <= 0) {
            energy_up = energy_down;  //用新的较小的能量作为Energy_up
            Delete_exchange_point();  //将交换后的点删除，并标记出来，
            main_black_point_number--;
            main_white_point_number--;
          } else if (judgement_times >= 1) {
            energy_up = energy_down;
            Delete_exchange_point();  //将交换后的点删除，并标记出来，
            main_black_point_number--;
            main_white_point_number--;
            judgement_times = 0;
          } else {  //说明当前位置的白点不能改变相位（占大头，不能交换），黑点需要改变，但需要与另一个白点进行交换（所以不delete）
            if (engerry_change_white > 0 && engerry_change_black < 0) {
              Recover_Multi_point_density_function();  //恢复到前一状态的多点密度函数
              Reverse_exchange_point();  //将不满足条件的交换点复原，
              Delete_unexchange_white_point();  //将交换后的点删除，
              main_white_point_number--;
            } else if (engerry_change_white < 0 && engerry_change_black > 0) {
              Recover_Multi_point_density_function();  //恢复到前一状态的多点密度函数
              Reverse_exchange_point();  //将不满足条件的交换点复原，
              Delete_unexchange_black_point();  //将交换后的点删除，
              main_black_point_number--;
            } else {
              Recover_Multi_point_density_function();  //恢复到前一状态的多点密度函数
              Reverse_exchange_point();  //将不满足条件的交换点复原，
              Delete_unexchange_two_phase_point();  //将交换后的点删除，
              main_black_point_number--;
              main_white_point_number--;
            }
          }
        }
        if (main_white_point_number == 0 || main_black_point_number == 0) {
          break;
        }
      }
      Delete_site_vector();  //将保持交换点的容器清空，以备下一轮循环，
      if (adjust_time == energy_up) {
        judgement_times++;
      }
      ////////////////////////////////////
      if (energy_jump == energy_up) {
        refuse_times_out++;
      } else {
        energy_jump = energy_up;
        refuse_times_out = 0;
      }
      // cout << "Energy_up:" << Energy_up << endl;
      ////////////////////////////////////
      if (refuse_times_out >= max_refuse_times_out) {
        break;
      }

      // if ((Energy_judge-Energy_up)<Energy_limit && refuse_times_out==0)
      if ((float)(energy_judge - energy_up) / energy_up < 0.005 &&
          refuse_times_out == 0) {
        break;
      } else {
        energy_judge = energy_up;
      }
      /////////////////////////////////////
    }
    Set_reconstruct_to_inital();  //将reconstructed中的中间重建结果转移到inital
    Putout_step_image(m_dstimg_path);  //输出原始图像对应网格图像
  }
  Set_reconstruct_to_final();
  Putout_final_image(m_dstimg_path);

  return true;
}

/////////////////////////////////1、确定最小网格/////////////////////////////////////////
void CAnnealing::Decide_min_grid(int& min_grid) {
  int i = 0, j = 0, p = 0, q = 0, sum = 0, k = 0, w = 0;
  int white_point = 0, black_point = 0;
  int loop_time = log((float)m_initial_width) / log(2.0);
  int temporay_value_1 = 0, temporay_value_2 = 0;
  bool judgement = false;

  for (k = 0; k != loop_time - 1; k++)  ////最小网格从4*4开始
  {
    temporay_value_1 = (int)pow(2.0, k);  ////相邻网格点之间的距离
    temporay_value_2 = m_initial_width / temporay_value_1;  ////网格的大小
    white_point = 0;
    black_point = 0;
    judgement = false;
    for (i = 0; i != temporay_value_2; i++) {
      for (j = 0; j != temporay_value_2; j++) {
        if (m_initial_imgvec[i * temporay_value_1][j * temporay_value_1] == 1) {
          white_point++;
        } else {
          black_point++;
        }
      }
    }
    for (
        i = 1; i != temporay_value_2 - 1;
        i++)  // 3*3模板的中心点，判断分级图像是否存在3*3区域为同一相，只要存在，则继续寻找
    {
      for (j = 1; j != temporay_value_2 - 1; j++) {
        sum = 0;
        for (p = -1; p != 2; p++) {
          for (q = -1; q != 2; q++) {
            sum = sum + m_initial_imgvec[(i + p) * temporay_value_1]
                                        [(j + q) * temporay_value_1];
          }
        }
        if (sum == 9 /*|| sum==0*/) {
          judgement = true;
        }
      }
    }
    if (judgement == false) {
      break;
    }
  }
  min_grid = temporay_value_2;
}

void CAnnealing::Transport_min_grid(int min_grid) {
  m_Rec_min_grid_size = min_grid;
}

inline unsigned __int32 rand32() {
  return ((rand() & 0x00007FE0) >> 5) + ((rand() & 0x00007FF0) << 6) +
         ((rand() & 0x00007FF0) << 17);
}

void CAnnealing::Set_inital_data_Recons() {
  //先定义一个三维容器，用于存放初始最小网格
  int i = 0, j = 0, k = 0, inital_white_point = 0;

  int new_inital_size = m_Rec_min_grid_size * m_multiple;  // 190113

  m_initial_rand_data.resize(
      new_inital_size);  // m_initial_rand_data初始最小容器随机赋值，
  for (i = 0; i != new_inital_size; ++i) {
    m_initial_rand_data[i].resize(new_inital_size);
    for (j = 0; j != new_inital_size; ++j) {
      m_initial_rand_data[i][j].resize(new_inital_size);
      for (k = 0; k != new_inital_size; k++) {
        m_initial_rand_data[i][j][k] = 2;
      }
    }
  }
  //按照孔隙度进行随机赋初值，白色用1表示，黑色用0表示
  int size_number = m_initial_height / m_Rec_min_grid_size;
  for (i = 0; i != m_Rec_min_grid_size; i++) {
    for (j = 0; j != m_Rec_min_grid_size; j++) {
      if (m_initial_imgvec[i * size_number][j * size_number] ==
          1)  // m_initial_imgvec存放原图容器
      {
        inital_white_point++;
      }
    }
  }
  int rand_site = 0;
  int counter_number =
      new_inital_size * new_inital_size *
      new_inital_size;  // 190113：原Rec_min_grid_size*Rec_min_grid_size*Rec_min_grid_size
  vector<int> point_x;
  vector<int> point_y;
  vector<int> point_z;
  vector<int>::iterator site_x = point_x.begin();
  vector<int>::iterator site_y = point_y.begin();
  vector<int>::iterator site_z = point_z.begin();
  for (i = 0; i != new_inital_size; i++)  // 190113
  {
    for (j = 0; j != new_inital_size; j++) {
      for (k = 0; k != new_inital_size; k++) {
        point_x.push_back(i);
        point_y.push_back(j);
        point_z.push_back(k);
      }
    }
  }

  //原图的孔隙点数相当于待重建三维结构一层的孔隙点数，乘以层数就为整个三维结构孔隙度
  for (i = 0;
       i != new_inital_size * inital_white_point * (m_multiple * m_multiple);
       i++)  //倍数要乘两次，是面积的比
  {
    rand_site = rand32() % counter_number;
    m_initial_rand_data[point_x[rand_site]][point_y[rand_site]]
                       [point_z[rand_site]] = 1;  // 1表示白色点，0表示黑色点
    site_x = point_x.begin() +
             rand_site;  //找到已经随机赋值的点，并删除，避免重复赋值
    site_y = point_y.begin() + rand_site;
    site_z = point_z.begin() + rand_site;
    point_x.erase(site_x);
    point_y.erase(site_y);
    point_z.erase(site_z);
    counter_number--;
  }
  point_x.clear();
  point_y.clear();
  point_z.clear();
  for (i = 0; i != new_inital_size; i++) {
    for (j = 0; j != new_inital_size; j++) {
      for (k = 0; k != new_inital_size; k++) {
        if (m_initial_rand_data[i][j][k] != 1) {
          m_initial_rand_data[i][j][k] = 0;
        }
      }
    }
  }
}

// m_initial_rand_data(初始最小容器)初始化(190113改)->(190212：将大孔三维结构信息也保存到最小容器中)
//大孔孔点所在位置初始化为300！——190212——重建的背景点可以在大孔孔点所在的位置，这样既能保证重建结果的孔隙度正确，且保证大孔结构不受影响
//大孔孔点位置只用限制为小孔孔点不能重建，但可以是小孔背景点（大孔孔点固定，不能被随机赋值）
void CAnnealing::Set_inital_data_2fuse3() {
  //先定义一个三维容器，用于存放初始最小网格
  int i = 0, j = 0, k = 0, inital_white_point = 0;

  int new_inital_size = m_Rec_min_grid_size * m_multiple;  // 190113
  int size_number = m_initial_height / m_Rec_min_grid_size;
  int b_pnum = 0;  //大孔孔点数                      //190212

  // m_initial_rand_data初始化为2和300（之前全是2）
  m_initial_rand_data.resize(
      new_inital_size);  // m_initial_rand_data初始最小容器随机赋值，
  for (i = 0; i != new_inital_size; ++i) {
    m_initial_rand_data[i].resize(new_inital_size);
    for (j = 0; j != new_inital_size; ++j) {
      m_initial_rand_data[i][j].resize(new_inital_size);
      for (k = 0; k != new_inital_size; k++) {
        if (m_big_3Dimgvec[i * size_number][j * size_number][k * size_number] !=
            0) {
          m_initial_rand_data[i][j][k] = 300;  // 190212：大孔孔点所在位置
          ++b_pnum;
        } else
          m_initial_rand_data[i][j][k] = 2;
      }
    }
  }
  //按照孔隙度进行随机赋初值，白色用1表示，黑色用0表示
  for (i = 0; i != m_Rec_min_grid_size; i++) {
    for (j = 0; j != m_Rec_min_grid_size; j++) {
      if (m_initial_imgvec[i * size_number][j * size_number] ==
          1)  // m_initial_imgvec存放原图容器（高分辨）
      {
        inital_white_point++;
      }
    }
  }
  int rand_site = 0;
  int counter_number = new_inital_size * new_inital_size *
                       new_inital_size /*-b_pnum*/;  // 190212
  vector<int> point_x;
  vector<int> point_y;
  vector<int> point_z;
  vector<int>::iterator site_x = point_x.begin();
  vector<int>::iterator site_y = point_y.begin();
  vector<int>::iterator site_z = point_z.begin();
  for (i = 0; i != new_inital_size; i++)  // 190113
  {
    for (j = 0; j != new_inital_size; j++) {
      for (k = 0; k != new_inital_size; k++) {
        // if (m_initial_rand_data[i][j][k]!=300)    //190212
        //{
        point_x.push_back(i);
        point_y.push_back(j);
        point_z.push_back(k);
        /*}*/
      }
    }
  }

  //
  //原图的孔隙点数相当于待重建三维结构一层的孔隙点数，乘以层数就为整个三维结构孔隙度
  for (i = 0;
       i != new_inital_size * inital_white_point * (m_multiple * m_multiple);
       i++) {
    rand_site = rand32() % counter_number;
    if (m_initial_rand_data[point_x[rand_site]][point_y[rand_site]]
                           [point_z[rand_site]] != 300)  // 190212
    {
      m_initial_rand_data[point_x[rand_site]][point_y[rand_site]]
                         [point_z[rand_site]] = 1;  // 1表示白色点，0表示黑色点
      site_x = point_x.begin() +
               rand_site;  //找到已经随机赋值的点，并删除，避免重复赋值
      site_y = point_y.begin() + rand_site;
      site_z = point_z.begin() + rand_site;
      point_x.erase(site_x);
      point_y.erase(site_y);
      point_z.erase(site_z);
      counter_number--;
    } else {
      --i;
      site_x =
          point_x.begin() +
          rand_site;  // 190213：找到“大孔孔点”所在位置，并删除，避免重复赋值
      site_y = point_y.begin() + rand_site;
      site_z = point_z.begin() + rand_site;
      point_x.erase(site_x);
      point_y.erase(site_y);
      point_z.erase(site_z);
      counter_number--;
    }
  }
  point_x.clear();
  point_y.clear();
  point_z.clear();
  for (i = 0; i != new_inital_size; i++) {
    for (j = 0; j != new_inital_size; j++) {
      for (k = 0; k != new_inital_size; k++) {
        if (m_initial_rand_data[i][j][k] !=
            1 /*&&m_initial_rand_data[i][j][k]!=300*/)  // 190212
        {
          m_initial_rand_data[i][j][k] = 0;
        }
      }
    }
  }
}

//为(new)m_vector_size(当前级训练图像的尺寸)赋值
void CAnnealing::Transfer_grid_size(int T_grid_size) {
  m_gridsize = T_grid_size;
  m_vector_size = m_Rec_min_grid_size * (int)pow(2.0, m_gridsize);
  m_new_vector_size = m_vector_size * m_multiple;  // 190113
}

// TI_code_value(存放训练图像的多点密度函数)
void CAnnealing::TI_Multi_point_density_function_p() {
  m_train_code_value.clear();  // TI_code_value存放训练图像的多点密度函数
                               // //将上一网格存储的多点密度函数清空
  int i = 0, j = 0;
  int reduce_size = m_initial_height / m_vector_size;  //缩小倍数
  int k = m_Rec_template_size / 2;
  vector<vector<int>> Temporary_vector;
  Temporary_vector.resize(m_vector_size);  //保存当前级数的TI
  for (i = 0; i != m_vector_size; ++i) {
    Temporary_vector[i].resize(m_vector_size);
  }

  //将对应点的原始图像放到中间过程容器
  for (i = 0; i != m_vector_size; i++) {
    for (j = 0; j != m_vector_size; j++) {
      if (m_initial_imgvec[i * reduce_size][j * reduce_size] == 1) {
        Temporary_vector[i][j] = 1;  //为了后面编码方便将255值变为1
      } else {
        Temporary_vector[i][j] = 0;
      }
    }
  }

  //计算此时缩小图像的多点密度函数
  int code_value = 0;  //模式编码值
  int q = 0, p = 0;
  for (i = 0; i != m_vector_size; ++i) {
    for (j = 0; j != m_vector_size; ++j) {
      code_value = 0;
      for (p = -k; p != k + 1; p++) {
        for (q = -k; q != k + 1; q++) {
          //这种编码方式能够保证编码不重复吗？最大编码值为511
          code_value =
              code_value * 2 +
              Temporary_vector[(i + p + m_vector_size) % m_vector_size]
                              [(j + q + m_vector_size) % m_vector_size];
        }
      }
      pair<map<int, int>::iterator, bool> pair_code_value =
          m_train_code_value.insert(make_pair(code_value, 1));
      if (!pair_code_value.second) {
        ++pair_code_value.first->second;
      }
    }
  }
}

//输出每一级原始图像对应网格图像
void CAnnealing::Putout_inital_image(const QString& filepath) {
  QDir qdir(filepath);
  if (!qdir.exists("initial")) qdir.mkdir("initial");
  QImage img(m_vector_size, m_vector_size, QImage::Format_Grayscale8);
  int i = 0, j = 0, k = 0, q = 0;
  k = m_initial_height / (m_vector_size);
  for (i = 0; i != m_vector_size; ++i) {
    uchar* lpix = img.scanLine(i);
    for (j = 0; j != m_vector_size; ++j) {
      q = m_initial_imgvec[i * k][j * k];
      *(lpix + j * 1) = (q ? 255 : 0);
    }
  }

  char name[100];
  sprintf(name, "The inital%d!.bmp", m_gridsize);  // grid_size就是T_grid_size
  QString respath = filepath;
  respath.append("/initial/").append(name);
  img.save(respath, nullptr, 100);
}

// 190212：m_reconstruct_vec第一次初始化时大孔孔点所在位置都初始化为0了！
void CAnnealing::
    Set_inital_to_reconstruct_uncondition_1()  //为最小网格对应的三维结构赋初值（即为Set_inital_data）
{
  int i = 0, j = 0, k = 0;
  m_reconstruct_vec.resize(m_new_vector_size);
  for (i = 0; i != m_new_vector_size; ++i) {
    m_reconstruct_vec[i].resize(m_new_vector_size);
    for (j = 0; j != m_new_vector_size; j++) {
      m_reconstruct_vec[i][j].resize(m_new_vector_size);
      for (k = 0; k != m_new_vector_size; k++) {
        m_reconstruct_vec[i][j][k] = m_initial_rand_data[i][j][k];
      }
    }
  }
}

void CAnnealing::Set_inital_to_reconstruct_uncondition_2_Recons() {
  int i = 0, j = 0, k = 0, p = 0, q = 0, n = 0;
  int sum = 0;
  int up_grid_white_number = 0;
  int current_grid_white_number = 0;
  int all_add_white_number = 0;
  int all_add_black_number = 0;
  int add_white_number_3D = 0;
  int add_white_number_2D = 0;
  // int add_white_number_1D=0;
  int white_point_number = 0;
  int distance_point = m_initial_width / m_vector_size;
  /////////////////////////////////////////////////给判决容器赋初值，将上层图像传递给重建容器；
  m_reconstruct_vec.resize(m_new_vector_size);  //(190113)存放重建结果容器
  m_preproc_vector.resize(
      m_new_vector_size);  //存放预处理结果容器，标注3*3全为黑或者全白的区域
  for (i = 0; i != m_new_vector_size; ++i) {
    m_reconstruct_vec[i].resize(m_new_vector_size);
    m_preproc_vector[i].resize(m_new_vector_size);
    for (j = 0; j != m_new_vector_size; j++) {
      m_reconstruct_vec[i][j].resize(m_new_vector_size);
      m_preproc_vector[i][j].resize(m_new_vector_size);
      for (k = 0; k != m_new_vector_size; k++) {
        m_preproc_vector[i][j][k] = 2;   /////初始化中间容器为2
        m_reconstruct_vec[i][j][k] = 2;  /////初始化中间容器为2
      }
    }
  }

  for (i = 0; i != m_vector_size; ++i) {
    for (j = 0; j != m_vector_size; ++j) {
      if (m_initial_imgvec[i * distance_point][j * distance_point] ==
          1)  /////计算本层原图白色点数
      {
        current_grid_white_number++;  //第一层的（16）之前已经单独拿出来算过了
      }
    }
  }
  ////////////////

  for (i = 0; i != m_new_vector_size / 2; ++i)  // 190113
  {
    for (j = 0; j != m_new_vector_size / 2; ++j) {
      for (k = 0; k != m_new_vector_size / 2; ++k) {
        m_reconstruct_vec[2 * i][2 * j][2 * k] = m_initial_rand_data
            [i][j]
            [k];  /////将上层重建的结果传递到对应网格点，m_initial_rand_data也是乘过倍数的了
        m_preproc_vector[2 * i][2 * j][2 * k] =
            m_initial_rand_data[i][j][k];  /////将上层重建的结果传递到对应网格点
        if (m_initial_rand_data[i][j][k] == 1) {
          up_grid_white_number++;  /////计算上层重建的结果孔隙相个数
        }
      }
    }
  }

  /////////////////////////////////////////////////////
  for (i = 0; i != m_new_vector_size; ++i)  // 190113
  {
    for (j = 0; j != m_new_vector_size; ++j) {
      for (k = 0; k != m_new_vector_size; ++k) {
        if (i % 2 == 1 && j % 2 == 1 &&
            k % 2 == 1)  // 3*3模板中心点长宽高坐标都必须为奇数！
        {
          sum = 0;  /////判断3*3*3区域是否全为1或者全0
          for (q = -1; q != 2; q++)  // i,j,k为奇数（奇+奇=偶、奇+偶=奇）
          {
            for (p = -1; p != 2; p++) {
              for (n = -1; n != 2; n++) {
                //下面的条件需要所有公式（1）为偶数，(i+q+m_new_vector_size)%m_new_vector_size使用的策略是周期边界条件，防止越界！
                if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((k + n + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0) {
                  // q、p、n需要为奇数
                  //满足条件的有(-1/1,-1/1,-1/1)共8个点，它们刚好为前一级映射过来的点（8个角）
                  sum = sum +
                        m_reconstruct_vec
                            [(i + q + m_new_vector_size) % m_new_vector_size]
                            [(j + p + m_new_vector_size) % m_new_vector_size]
                            [(k + n + m_new_vector_size) % m_new_vector_size];
                }
              }
            }
          }
          //注意：m_reconstruct_vec虽初始化值为2，但是一部分点（隔点插值0、2、4...）的值又根据前一级重建结果被初始化为0或1
          //这里sum是统计的前一级映射过来的像素点！
          if (sum == 8)  /////全1时，除该区域中心点外，其余点全赋值为1
          {
            for (q = -1; q != 2; q++)  // i,j,k为奇数（奇+奇=偶、奇+偶=奇）
            {
              for (p = -1; p != 2; p++) {
                for (n = -1; n != 2; n++) {
                  //有一个公式满足和为奇数即可
                  if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1 ||
                      ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1 ||
                      ((k + n + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1) {
                    m_preproc_vector
                        [(i + q + m_new_vector_size) % m_new_vector_size]
                        [(j + p + m_new_vector_size) % m_new_vector_size]
                        [(k + n + m_new_vector_size) % m_new_vector_size] = 31;
                  }
                }
              }
            }
          }

          else if (sum == 0) {
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                for (n = -1; n != 2; n++) {
                  if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1 ||
                      ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1 ||
                      ((k + n + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1) {
                    m_preproc_vector
                        [(i + q + m_new_vector_size) % m_new_vector_size]
                        [(j + p + m_new_vector_size) % m_new_vector_size]
                        [(k + n + m_new_vector_size) % m_new_vector_size] = 30;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  // current_grid_white_number为本级训练图像中的白点数(190113)，up_grid_white_number为前一级重建结果中的白点数
  all_add_white_number = m_new_vector_size * current_grid_white_number *
                             (m_multiple * m_multiple) -
                         up_grid_white_number;  /////本级网格需要添加白色点个数
  all_add_black_number =
      m_new_vector_size *
      (m_new_vector_size * m_new_vector_size -
       current_grid_white_number * (m_multiple * m_multiple));
  int white_point_number_3D = 0;
  int black_point_number_3D = 0;
  int white_point_number_2D = 0;
  int black_point_number_2D = 0;
  // int white_point_number_1D=0;
  // int black_point_number_1D=0;

  for (i = 0; i != m_new_vector_size; ++i)  /////统计上面添加1和0的个数
  {
    for (j = 0; j != m_new_vector_size; ++j) {
      for (k = 0; k != m_new_vector_size; ++k) {
        // m_preproc_vector中为黑白点可能存在的位置
        //根据相的聚合规律，前一级全为白点/黑点的模式，在下一级的扩展模式中扩展点大概率也是白点/黑点
        if (m_preproc_vector[i][j][k] == 31)  // 3*3*3区域全为1
        {
          white_point_number_3D++;
        } else if (m_preproc_vector[i][j][k] == 30)  // 3*3*3区域全为0
        {
          black_point_number_3D++;
        }
      }
    }
  }

  int point_x = 0;
  int point_y = 0;
  int point_z = 0;

  //若实际待添加的黑白点数>=根据聚合规律假设的黑白点数，则直接将假设的黑白点位置直接按假设赋值
  if (all_add_white_number >= white_point_number_3D &&
      all_add_black_number >=
          black_point_number_3D)  /////将m_reconstruct_vec对应3D网格点赋值1和0
  {
    for (i = 0; i != m_new_vector_size;
         ++i)  //先将3*3*3区域判断的点直接将这些满足条件的数据赋值
    {
      for (j = 0; j != m_new_vector_size; ++j) {
        for (k = 0; k != m_new_vector_size; ++k) {
          if (m_preproc_vector[i][j][k] == 31) {
            m_reconstruct_vec[i][j][k] = 1;
          } else if (m_preproc_vector[i][j][k] == 30) {
            m_reconstruct_vec[i][j][k] = 0;
          }
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////以上部分是处理上级网格3*3*3区域为全1和全0情况
    ////////////////////////////////////////////////////////////////////////////以下部分是处理上级网格2*2区域为全1和全0情况
    for (i = 0; i != m_new_vector_size; ++i)  //判断全1或者全0的2*2*2区域点个数
    {
      for (j = 0; j != m_new_vector_size; ++j) {
        for (k = 0; k != m_new_vector_size; ++k) {
          // i为偶数，j、k为奇数，且像素点还未赋值（仍是初始值2）
          if (i % 2 == 0 && j % 2 == 1 && k % 2 == 1 &&
              m_preproc_vector[i][j][k] == 2) {
            sum = 0;
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                //只考虑j、k，需要q、p为奇数
                if (((j + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((k + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0) {
                  sum = sum + m_reconstruct_vec[i][(j + q + m_new_vector_size) %
                                                   m_new_vector_size]
                                               [(k + p + m_new_vector_size) %
                                                m_new_vector_size];
                }
              }
            }
            if (sum == 4) {
              m_preproc_vector[i][j][k] = 21;
              white_point_number_2D++;
            }

            else if (sum == 0) {
              m_preproc_vector[i][j][k] = 20;
              black_point_number_2D++;
            }
          }

          // k为偶数，j、i为奇数，且像素点还未赋值（仍是初始值2）
          else if (i % 2 == 1 && j % 2 == 1 && k % 2 == 0 &&
                   m_preproc_vector[i][j][k] == 2) {
            sum = 0;
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0) {
                  sum = sum + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                m_new_vector_size]
                                               [(j + p + m_new_vector_size) %
                                                m_new_vector_size][k];
                }
              }
            }
            if (sum == 4) {
              m_preproc_vector[i][j][k] = 21;
              white_point_number_2D++;
            }

            else if (sum == 0) {
              m_preproc_vector[i][j][k] = 20;
              black_point_number_2D++;
            }
          }

          // j为偶数，k、i为奇数，且像素点还未赋值（仍是初始值2）
          else if (i % 2 == 1 && j % 2 == 0 && k % 2 == 1 &&
                   m_preproc_vector[i][j][k] == 2) {
            sum = 0;
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((k + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0) {
                  sum = sum + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                m_new_vector_size][j]
                                               [(k + p + m_new_vector_size) %
                                                m_new_vector_size];
                }
              }
            }
            if (sum == 4) {
              m_preproc_vector[i][j][k] = 21;
              white_point_number_2D++;
            }

            else if (sum == 0) {
              m_preproc_vector[i][j][k] = 20;
              black_point_number_2D++;
            }
          }
        }
      }
    }
    /////第二次2*2中心点直接赋值满足孔隙度
    if (all_add_white_number >=
            (white_point_number_3D + white_point_number_2D) &&
        all_add_black_number >=
            (black_point_number_3D + black_point_number_2D)) {
      for (i = 0; i != m_new_vector_size; ++i)  //先直接将这些满足条件的数据赋值
      {
        for (j = 0; j != m_new_vector_size; ++j) {
          for (k = 0; k != m_new_vector_size; ++k) {
            if (m_preproc_vector[i][j][k] == 21) {
              m_reconstruct_vec[i][j][k] = 1;
            } else if (m_preproc_vector[i][j][k] == 20) {
              m_reconstruct_vec[i][j][k] = 0;
            }
          }
        }
      }

      ///////////////第三次判断2*2区域是否全为0或者1
      add_white_number_2D = (all_add_white_number - white_point_number_3D -
                             white_point_number_2D);  //，然后从剩余部分随机赋值
      while (add_white_number_2D != 0) {
        point_x = rand32() % m_new_vector_size;
        point_y = rand32() % m_new_vector_size;
        point_z = rand32() % m_new_vector_size;
        if ((point_y % 2 == 1 || point_x % 2 == 1 || point_z % 2 == 1) &&
            m_preproc_vector[point_x][point_y][point_z] == 2 &&
            m_reconstruct_vec[point_x][point_y][point_z] == 2) {
          m_reconstruct_vec[point_x][point_y][point_z] = 1;
          add_white_number_2D--;
        }
      }

      for (i = 0; i != m_new_vector_size; i++) {
        for (j = 0; j != m_new_vector_size; j++) {
          for (k = 0; k != m_new_vector_size; k++) {
            if (m_reconstruct_vec[i][j][k] == 2) {
              m_reconstruct_vec[i][j][k] = 0;
            }
          }
        }
      }
    }

    else  /////第二次2*2*2中心点直接赋值不满足孔隙度时，回退到上一步3*3*3设置状态
    {
      cout << "Unexpected condition second!!" << endl;
      add_white_number_3D = all_add_white_number - white_point_number_3D;
      while (add_white_number_3D != 0) {
        point_x = rand32() % m_new_vector_size;
        point_y = rand32() % m_new_vector_size;
        point_z = rand32() % m_new_vector_size;
        if ((point_y % 2 == 1 || point_x % 2 == 1 || point_z % 2 == 1) &&
            m_preproc_vector[point_x][point_y][point_z] == 2) {
          m_reconstruct_vec[point_x][point_y][point_z] = 1;
          add_white_number_3D--;
        }
      }
      for (i = 0; i != m_new_vector_size; i++) {
        for (j = 0; j != m_new_vector_size; j++) {
          for (k = 0; k != m_new_vector_size; k++) {
            if (m_reconstruct_vec[i][j][k] == 2) {
              m_reconstruct_vec[i][j][k] = 0;
            }
          }
        }
      }
    }
  }

  //实际待添加的黑白点数 < 根据聚合规律假设的黑白点数（3D情况下填的都已经多了）
  else  /////直接按照需要随机赋值
  {
    cout << "Unexpected condition first!!" << endl;
    while (all_add_white_number != 0) {
      point_x = rand32() % m_new_vector_size;
      point_y = rand32() % m_new_vector_size;
      point_z = rand32() % m_new_vector_size;
      if (point_y % 2 == 1 || point_x % 2 == 1 || point_z % 2 == 1) {
        m_reconstruct_vec[point_x][point_y][point_z] = 1;
        all_add_white_number--;
      }
    }
    for (i = 0; i != m_new_vector_size; i++) {
      for (j = 0; j != m_new_vector_size; j++) {
        for (k = 0; k != m_new_vector_size; k++) {
          if (m_reconstruct_vec[i][j][k] == 2) {
            m_reconstruct_vec[i][j][k] = 0;
          }
          m_preproc_vector[i][j][k] =
              m_reconstruct_vec[i][j]
                               [k];  /////最后将m_preproc_vector作为指示容器
        }
      }
    }
  }
  /////将m_preproc_vector设置为指示器，除了上级保留的，聚合规律赋值的，其它为2
  for (i = 0; i != m_new_vector_size; i++) {
    for (j = 0; j != m_new_vector_size; j++) {
      for (k = 0; k != m_new_vector_size; k++) {
        if (m_preproc_vector[i][j][k] != 2 && m_reconstruct_vec[i][j][k] == 1) {
          m_preproc_vector[i][j][k] = 1;  //之前的31，21
        } else if (m_preproc_vector[i][j][k] != 2 &&
                   m_reconstruct_vec[i][j][k] == 0) {
          m_preproc_vector[i][j][k] = 0;
        }
      }
    }
  }
}

// m_preproc_vector
void CAnnealing::Set_inital_to_reconstruct_uncondition_2_2fuse3() {
  int i = 0, j = 0, k = 0, p = 0, q = 0, n = 0;
  int sum = 0;
  int up_grid_white_number = 0;
  int current_grid_white_number = 0;
  int all_add_white_number = 0;
  int all_add_black_number = 0;
  int add_white_number_3D = 0;
  int add_white_number_2D = 0;
  // int add_white_number_1D=0;
  int white_point_number = 0;
  int distance_point = m_initial_width / m_vector_size;
  /////////////////////////////////////////////////给判决容器赋初值，将上层图像传递给重建容器；
  m_reconstruct_vec.resize(m_new_vector_size);  //(190113)存放重建结果容器
  m_preproc_vector.resize(
      m_new_vector_size);  //存放预处理结果容器，标注3*3全为黑或者全白的区域
  for (i = 0; i != m_new_vector_size; ++i) {
    m_reconstruct_vec[i].resize(m_new_vector_size);
    m_preproc_vector[i].resize(m_new_vector_size);
    for (j = 0; j != m_new_vector_size; j++) {
      m_reconstruct_vec[i][j].resize(m_new_vector_size);
      m_preproc_vector[i][j].resize(m_new_vector_size);
      for (k = 0; k != m_new_vector_size; k++) {
        if (m_big_3Dimgvec[i * distance_point][j * distance_point]
                          [k * distance_point] != 0) {
          m_preproc_vector[i][j][k] = 300;  /////190212
          m_reconstruct_vec[i][j][k] = 2;  /////初始化中间容器"大孔孔点"为300
        } else {
          m_preproc_vector[i][j][k] = 2;   /////初始化中间容器为2
          m_reconstruct_vec[i][j][k] = 2;  /////初始化中间容器为2
        }
      }
    }
  }
  for (i = 0; i != m_vector_size; ++i) {
    for (j = 0; j != m_vector_size; ++j) {
      if (m_initial_imgvec[i * distance_point][j * distance_point] ==
          1)  /////计算本层原图白色点数
      {
        current_grid_white_number++;
      }
    }
  }
  ////////////////

  for (i = 0; i != m_new_vector_size / 2; ++i)  // 190113
  {
    for (j = 0; j != m_new_vector_size / 2; ++j) {
      for (k = 0; k != m_new_vector_size / 2; ++k) {
        ///////190212
        if (m_preproc_vector[2 * i][2 * j][2 * k] != 300) {
          m_reconstruct_vec[2 * i][2 * j][2 * k] =
              m_initial_rand_data[i][j]
                                 [k];  /////将上层重建的结果传递到对应网格点
          m_preproc_vector[2 * i][2 * j][2 * k] =
              m_initial_rand_data[i][j]
                                 [k];  /////将上层重建的结果传递到对应网格点
          if (m_initial_rand_data[i][j][k] == 1) {
            up_grid_white_number++;  /////计算上层重建的结果孔隙相（190212：不与下层大孔孔点重合）的个数
          }
        } else {
          // if (m_initial_rand_data[i][j][k]==0)
          // //190213：大孔孔点所在的位置值也为0（去掉这一条件，将大孔孔 {
          ////点所在位置设为已知的背景条件点，对比实验结果！）
          m_reconstruct_vec[2 * i][2 * j][2 * k] = 0;
          /*}*/
        }
      }
    }
  }

  /////////////////////////////////////////////////////
  for (i = 0; i != m_new_vector_size;
       ++i)  // 190113（处理奇数x、y、z对应的像素点！）
  {
    for (j = 0; j != m_new_vector_size; ++j) {
      for (k = 0; k != m_new_vector_size; ++k) {
        if (i % 2 == 1 && j % 2 == 1 &&
            k % 2 == 1)  // 3*3模板中心点长宽高坐标都必须为奇数！
        {
          sum = 0;  /////判断3*3*3区域是否全为1或者全0
          for (q = -1; q != 2; q++)  // i,j,k为奇数（奇+奇=偶、奇+偶=奇）
          {
            for (p = -1; p != 2; p++) {
              for (n = -1; n != 2; n++) {
                //下面的条件需要所有公式（1）为偶数，(i+q+m_new_vector_size)%m_new_vector_size使用的策略是周期边界条件，防止越界！
                if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((k + n + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0) {
                  // q、p、n需要为奇数
                  //满足条件的有(-1/1,-1/1,-1/1)共8个点，它们刚好为前一级映射过来的点
                  sum = sum +
                        m_reconstruct_vec
                            [(i + q + m_new_vector_size) % m_new_vector_size]
                            [(j + p + m_new_vector_size) % m_new_vector_size]
                            [(k + n + m_new_vector_size) % m_new_vector_size];
                }
              }
            }
          }
          //注意：m_reconstruct_vec虽初始化值为2，但是一部分点（隔点插值0、2、4...）的值又根据前一级重建结果被初始化为0或1
          //这里sum是统计的前一级映射过来的像素点！
          if (sum == 8)  /////全1时，除该区域中心点外，其余点全赋值为1
          {
            for (q = -1; q != 2; q++)  // i,j,k为奇数（奇+奇=偶、奇+偶=奇）
            {
              for (p = -1; p != 2; p++) {
                for (n = -1; n != 2; n++) {
                  //有一个公式满足和为奇数即可
                  if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1 ||
                      ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1 ||
                      ((k + n + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1) {
                    // q、p、n至少有一个为偶数，即0！
                    //即满足由前一级映射过来的点全为1时，3*3*3模板中所有值为2的点（扩展点）
                    if (m_preproc_vector
                            [(i + q + m_new_vector_size) % m_new_vector_size]
                            [(j + p + m_new_vector_size) % m_new_vector_size]
                            [(k + n + m_new_vector_size) % m_new_vector_size] !=
                        300)  // 190212
                    {
                      m_preproc_vector
                          [(i + q + m_new_vector_size) % m_new_vector_size]
                          [(j + p + m_new_vector_size) % m_new_vector_size]
                          [(k + n + m_new_vector_size) % m_new_vector_size] =
                              31;
                    }
                  }
                }
              }
            }
          } else if (sum == 0)  /////全0时，除该区域中心点外，其余点全赋值为0
          {
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                for (n = -1; n != 2; n++) {
                  if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1 ||
                      ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1 ||
                      ((k + n + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1) {
                    // if
                    // (m_preproc_vector[(i+q+m_new_vector_size)%m_new_vector_size][(j+p+m_new_vector_size)%m_new_vector_size][(k+n+m_new_vector_size)%m_new_vector_size]!=300)
                    // //190212：可以不要这个条件吧？大孔孔点可以在小孔结构的背景处
                    //{
                    m_preproc_vector
                        [(i + q + m_new_vector_size) % m_new_vector_size]
                        [(j + p + m_new_vector_size) % m_new_vector_size]
                        [(k + n + m_new_vector_size) % m_new_vector_size] = 30;
                    //}
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  // current_grid_white_number为本级训练图像中的白点数(190113)，up_grid_white_number为前一级重建结果中的白点数
  all_add_white_number = m_new_vector_size * current_grid_white_number *
                             (m_multiple * m_multiple) -
                         up_grid_white_number;  /////本级网格需要添加白色点个数
  all_add_black_number =
      m_new_vector_size *
      (m_new_vector_size * m_new_vector_size -
       current_grid_white_number * (m_multiple * m_multiple));
  int white_point_number_3D = 0;
  int black_point_number_3D = 0;
  int white_point_number_2D = 0;
  int black_point_number_2D = 0;
  // int white_point_number_1D=0;
  // int black_point_number_1D=0;

  for (i = 0; i != m_new_vector_size; ++i)  /////统计上面添加1和0的个数
  {
    for (j = 0; j != m_new_vector_size; ++j) {
      for (k = 0; k != m_new_vector_size; ++k) {
        // m_preproc_vector中为黑白点可能存在的位置
        //根据相的聚合规律，前一级全为白点/黑点的模式，在下一级的扩展模式中扩展点大概率也是白点/黑点
        if (m_preproc_vector[i][j][k] == 31)  // 3*3*3区域全为1
        {
          white_point_number_3D++;
        } else if (m_preproc_vector[i][j][k] == 30)  // 3*3*3区域全为0
        {
          black_point_number_3D++;
        }
      }
    }
  }

  int point_x = 0;
  int point_y = 0;
  int point_z = 0;

  //若实际待添加的黑白点数>=根据聚合规律假设的黑白点数，则直接将假设的黑白点位置直接按假设赋值
  if (all_add_white_number >= white_point_number_3D &&
      all_add_black_number >=
          black_point_number_3D)  /////将m_reconstruct_vec对应3D网格点赋值1和0
  {
    for (i = 0; i != m_new_vector_size;
         ++i)  //先将3*3*3区域判断的点直接将这些满足条件的数据赋值
    {
      for (j = 0; j != m_new_vector_size; ++j) {
        for (k = 0; k != m_new_vector_size; ++k) {
          if (m_preproc_vector[i][j][k] == 31) {
            m_reconstruct_vec[i][j][k] = 1;
          } else if (m_preproc_vector[i][j][k] == 30) {
            m_reconstruct_vec[i][j][k] = 0;
          }
        }
      }
    }
    //下面：处理偶数层
    ////////////////////////////////////////////////////////////////////////////以上部分是处理上级网格3*3*3区域为全1和全0情况
    ////////////////////////////////////////////////////////////////////////////以下部分是处理上级网格2*2区域为全1和全0情况
    for (i = 0; i != m_new_vector_size; ++i)  //判断全1或者全0的2*2*2区域点个数
    {
      for (j = 0; j != m_new_vector_size; ++j) {
        for (k = 0; k != m_new_vector_size; ++k) {
          // i为偶数，j、k为奇数，且像素点还未赋值（仍是初始值2||(190212：300)）
          if (i % 2 == 0 && j % 2 == 1 && k % 2 == 1 &&
              (m_preproc_vector[i][j][k] == 2 ||
               m_preproc_vector[i][j][k] == 300)) {
            sum = 0;
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                //只考虑j、k，需要q、p为奇数
                if (((j + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((k + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0) {
                  sum = sum + m_reconstruct_vec[i][(j + q + m_new_vector_size) %
                                                   m_new_vector_size]
                                               [(k + p + m_new_vector_size) %
                                                m_new_vector_size];
                }
              }
            }
            if (sum == 4) {
              if (m_preproc_vector[i][j][k] !=
                  300)  // 190213：是否有点问题？Pre大孔所在的位置可能在上一部分已经赋值为30？！
              {  //没问题！因为上面控制了Pre的值只能为2或300！
                m_preproc_vector[i][j][k] = 21;
                white_point_number_2D++;
              }
            } else if (sum == 0) {
              m_preproc_vector[i][j][k] = 20;
              black_point_number_2D++;
            }
          }

          // k为偶数，j、i为奇数，且像素点还未赋值（仍是初始值2||(190212：300)）
          else if (i % 2 == 1 && j % 2 == 1 && k % 2 == 0 &&
                   (m_preproc_vector[i][j][k] == 2 ||
                    m_preproc_vector[i][j][k] == 300)) {
            sum = 0;
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0) {
                  sum = sum + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                m_new_vector_size]
                                               [(j + p + m_new_vector_size) %
                                                m_new_vector_size][k];
                }
              }
            }
            if (sum == 4) {
              if (m_preproc_vector[i][j][k] != 300)  // 190212
              {
                m_preproc_vector[i][j][k] = 21;
                white_point_number_2D++;
              }
            } else if (sum == 0) {
              m_preproc_vector[i][j][k] = 20;
              black_point_number_2D++;
            }
          }

          // j为偶数，k、i为奇数，且像素点还未赋值（仍是初始值2||(190212：300)）
          else if (i % 2 == 1 && j % 2 == 0 && k % 2 == 1 &&
                   (m_preproc_vector[i][j][k] == 2 ||
                    m_preproc_vector[i][j][k] == 300)) {
            sum = 0;
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((k + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0) {
                  sum = sum + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                m_new_vector_size][j]
                                               [(k + p + m_new_vector_size) %
                                                m_new_vector_size];
                }
              }
            }
            if (sum == 4) {
              if (m_preproc_vector[i][j][k] != 300)  // 190212
              {
                m_preproc_vector[i][j][k] = 21;
                white_point_number_2D++;
              }

            } else if (sum == 0) {
              m_preproc_vector[i][j][k] = 20;
              black_point_number_2D++;
            }
          }
        }
      }
    }
    /////第二次2*2中心点直接赋值满足孔隙度
    if (all_add_white_number >=
            (white_point_number_3D + white_point_number_2D) &&
        all_add_black_number >=
            (black_point_number_3D + black_point_number_2D)) {
      for (i = 0; i != m_new_vector_size; ++i)  //先直接将这些满足条件的数据赋值
      {
        for (j = 0; j != m_new_vector_size; ++j) {
          for (k = 0; k != m_new_vector_size; ++k) {
            if (m_preproc_vector[i][j][k] == 21) {
              m_reconstruct_vec[i][j][k] = 1;
            } else if (m_preproc_vector[i][j][k] == 20) {
              m_reconstruct_vec[i][j][k] = 0;
            }
          }
        }
      }

      ///////////////第三次判断2*2区域是否全为0或者1
      add_white_number_2D = (all_add_white_number - white_point_number_3D -
                             white_point_number_2D);  //然后从剩余部分随机赋值
      while (add_white_number_2D != 0) {
        point_x = rand32() % m_new_vector_size;
        point_y = rand32() % m_new_vector_size;
        point_z = rand32() % m_new_vector_size;
        if ((point_y % 2 == 1 || point_x % 2 == 1 || point_z % 2 == 1) &&
            m_preproc_vector[point_x][point_y][point_z] == 2 &&
            m_reconstruct_vec[point_x][point_y][point_z] == 2) {
          m_reconstruct_vec[point_x][point_y][point_z] = 1;
          add_white_number_2D--;
        }
      }

      for (i = 0; i != m_new_vector_size; i++) {
        for (j = 0; j != m_new_vector_size; j++) {
          for (k = 0; k != m_new_vector_size; k++) {
            if (m_reconstruct_vec[i][j][k] == 2) {
              m_reconstruct_vec[i][j][k] = 0;
            }
          }
        }
      }
    }

    else  /////第二次2*2*2中心点直接赋值不满足孔隙度时，回退到上一步3*3*3设置状态
    {
      cout << "Unexpected condition second!!" << endl;
      add_white_number_3D = all_add_white_number - white_point_number_3D;
      while (add_white_number_3D != 0) {
        point_x = rand32() % m_new_vector_size;
        point_y = rand32() % m_new_vector_size;
        point_z = rand32() % m_new_vector_size;
        if ((point_y % 2 == 1 || point_x % 2 == 1 || point_z % 2 == 1) &&
            m_preproc_vector[point_x][point_y][point_z] == 2) {
          m_reconstruct_vec[point_x][point_y][point_z] = 1;
          add_white_number_3D--;
        }
      }
      for (i = 0; i != m_new_vector_size; i++) {
        for (j = 0; j != m_new_vector_size; j++) {
          for (k = 0; k != m_new_vector_size; k++) {
            if (m_reconstruct_vec[i][j][k] == 2) {
              m_reconstruct_vec[i][j][k] = 0;
            }
          }
        }
      }
    }
  }

  else  /////直接按照需要随机赋值
  {
    cout << "Unexpected condition first!!" << endl;
    while (all_add_white_number != 0) {
      point_x = rand32() % m_new_vector_size;
      point_y = rand32() % m_new_vector_size;
      point_z = rand32() % m_new_vector_size;
      if ((point_y % 2 == 1 || point_x % 2 == 1 || point_z % 2 == 1) &&
          m_preproc_vector[point_x][point_y][point_z] != 300)  // 190212
      {
        m_reconstruct_vec[point_x][point_y][point_z] = 1;
        all_add_white_number--;
      }
    }
    for (i = 0; i != m_new_vector_size; i++) {
      for (j = 0; j != m_new_vector_size; j++) {
        for (k = 0; k != m_new_vector_size; k++) {
          if (m_reconstruct_vec[i][j][k] == 2) {
            m_reconstruct_vec[i][j][k] = 0;
          }
          m_preproc_vector[i][j][k] =
              m_reconstruct_vec[i][j]
                               [k];  /////最后将m_preproc_vector作为指示容器
        }
      }
    }
  }
  /////将m_preproc_vector设置为指示器
  for (i = 0; i != m_new_vector_size; i++) {
    for (j = 0; j != m_new_vector_size; j++) {
      for (k = 0; k != m_new_vector_size; k++) {
        if (m_preproc_vector[i][j][k] != 2 &&
            m_preproc_vector[i][j][k] != 300 &&
            m_reconstruct_vec[i][j][k] ==
                1) {  // 190213：该条件好像不需要，因为reconst为1的位置一定不是大孔孔点位置
          m_preproc_vector[i][j][k] = 1;
        } else if (m_preproc_vector[i][j][k] != 2 &&
                   m_reconstruct_vec[i][j][k] == 0) {
          m_preproc_vector[i][j][k] = 0;
        }
      }
    }
  }
}

// Reconstruct_code_value(存放重建图像的多点密度函数)——190212：m_reconstruct_vec中没有300值！
void CAnnealing::Recstruct_Multi_point_density_function_p() {
  m_reconstruct_code_value.clear();  //将上一网格存储的多点密度函数清空
  int i = 0, j = 0, k = 0, q = 0, p = 0, r = 0;
  int s = m_Rec_template_size / 2;
  pair<map<int, int>::iterator, bool>
      pair_code_value;  //计算此时重建图像的多点密度函数
  int code_value = 0;   //模式编码值
  for (k = 0; k != m_new_vector_size; ++k)  //对三个面的模式密度进行统计
  {
    for (j = 0; j != m_new_vector_size; ++j) {
      for (i = 0; i != m_new_vector_size; ++i) {
        code_value = 0;
        for (p = -s; p != s + 1; p++) {
          for (q = -s; q != s + 1; q++) {
            code_value = code_value * 2 +
                         m_reconstruct_vec[k][(i + p + m_new_vector_size) %
                                              m_new_vector_size]
                                          [(j + q + m_new_vector_size) %
                                           m_new_vector_size];
          }
        }
        pair_code_value =
            m_reconstruct_code_value.insert(make_pair(code_value, 1));
        if (!pair_code_value.second) {
          ++pair_code_value.first->second;
        }
        code_value = 0;
        for (p = -s; p != s + 1; p++) {
          for (q = -s; q != s + 1; q++) {
            code_value = code_value * 2 +
                         m_reconstruct_vec[(i + p + m_new_vector_size) %
                                           m_new_vector_size][k]
                                          [(j + q + m_new_vector_size) %
                                           m_new_vector_size];
          }
        }
        pair_code_value =
            m_reconstruct_code_value.insert(make_pair(code_value, 1));
        if (!pair_code_value.second) {
          ++pair_code_value.first->second;
        }
        code_value = 0;
        for (p = -s; p != s + 1; p++) {
          for (q = -s; q != s + 1; q++) {
            code_value = code_value * 2 +
                         m_reconstruct_vec[(i + p + m_new_vector_size) %
                                           m_new_vector_size]
                                          [(j + q + m_new_vector_size) %
                                           m_new_vector_size][k];
          }
        }
        pair_code_value =
            m_reconstruct_code_value.insert(make_pair(code_value, 1));
        if (!pair_code_value.second) {
          ++pair_code_value.first->second;
        }
      }
    }
  }
}

void CAnnealing::LoadInitialImg(const QString& imgpath) {
  QImage* img = new QImage(imgpath);
  QImage grayimg = img->convertToFormat(QImage::Format_Grayscale8);
  delete img;

  int w = grayimg.width();
  int h = grayimg.height();
  m_initial_height = h;
  m_initial_width = w;
  m_initial_imgvec.resize(h);
  for (int i = 0; i < h; i++) {
    const uchar* lpix = grayimg.scanLine(i);
    m_initial_imgvec[i].resize(w);
    for (int j = 0; j < w; j++) {
      int pix = *(lpix + j * 1);
      m_initial_imgvec[i][j] = (pix ? 1 : 0);
    }
  }

  //注意,原核心算法中的加载如下,有误
  /*
  int i=0,j=0,k=0;
        m_initial_imgvec.resize(width);
        for (j=0;j!=width;j++)
        {
                m_initial_imgvec[j].resize(height);
                for (i=0;i!=height;i++)
                {
                        k= img->imageData[j*width+i];
                        if (k==0)
                        {
                                m_initial_imgvec[j][i]=0;
                        }
                        else
                        {
                                m_initial_imgvec[j][i]=1;
                        }
                }
        }
  */
}

bool CAnnealing::Load3DImg(const QFileInfoList& filelist) {
  //三维体长宽高应该一致
  if (filelist.size() != m_final_imgsize) {
    QMessageBox msgBox;
    msgBox.setText(TR("打开序列图张数应等于图片尺寸,请检查文件夹!"));
    msgBox.exec();
    return false;
  }

  //加载序列图
  QImage img;
  m_big_3Dimgvec.resize(m_final_imgsize);
  int index = 0;
  for (auto itr = filelist.begin(); itr != filelist.end(); itr++, index++) {
    m_big_3Dimgvec[index].resize(m_final_imgsize);
    //将文件路径添加到m_filespath
    QString tempPath = itr->absoluteFilePath();
    img.load(tempPath);
    QImage grayimg = img.convertToFormat(QImage::Format_Grayscale8);
    for (int i = 0; i < m_final_imgsize; i++) {
      const uchar* lpix = grayimg.scanLine(i);
      m_big_3Dimgvec[index][i].resize(m_final_imgsize);
      for (int j = 0; j < m_final_imgsize; j++) {
        int pix = *(lpix + j * 1);
        m_big_3Dimgvec[index][i][j] = (pix ? 255 : 0);
      }
    }
  }
  return true;
}

void CAnnealing::Calculate_E_differ(int& E_differ) {
  int i = 0;
  E_differ = 0;
  for (i = 0; i != m_sizenum; i++)  // size_number模板对应大小编码最大值
  {
    //(190113)3表示x、y、z三个面，m_vector_size为三维层数，TI_code_value为单层的模式密度函数
    E_differ = E_differ + abs(3 * m_new_vector_size * m_train_code_value[i] *
                                  (m_multiple * m_multiple) -
                              m_reconstruct_code_value[i]);
  }
  m_Enerry_origial = E_differ;
}

// white_point_number=Main_white_point_number
// 190213：由于m_reconstruct_vec中没有对大孔孔点的标志(300)，必须限定大孔孔点位置只能为背景点且不能参与交换！
void CAnnealing::Select_inital_exchange_point_uncondition_2fuse3(
    int& white_point_number, int& black_point_number) {
  int i = 0, j = 0, k = 0;
  int p = 0, q = 0, g = 0;
  white_point_number = 0;
  black_point_number = 0;

  int distance_point = m_initial_width / m_vector_size;  // 190213

  for (i = 0; i != m_new_vector_size; i++) {
    for (j = 0; j != m_new_vector_size; j++) {
      for (k = 0; k != m_new_vector_size; k++) {
        if (m_reconstruct_vec[i][j][k] == 1)  //存放重建结果容器
        {
          m_white_posx.push_back(i);
          m_white_posy.push_back(j);
          m_white_posz.push_back(k);
          ++white_point_number;
        } else if (
            m_big_3Dimgvec[i * distance_point][j * distance_point]
                          [k * distance_point] == 0 &&
            m_reconstruct_vec[i][j][k] ==
                0) {  // 190213：只有非大孔孔点所在位置的背景点才能参与交换！
          m_black_posx.push_back(i);
          m_black_posy.push_back(j);
          m_black_posz.push_back(k);
          ++black_point_number;
        }
      }
    }
  }
  m_class_white_point_number = white_point_number;
  m_class_black_point_number = black_point_number;
}

void CAnnealing::Select_final_exchange_point_uncondition_Recons(
    int& white_point_number, int& black_point_number) {
  int i = 0, j = 0, k = 0, p = 0, q = 0, g = 0, n = 0;
  vector<vector<vector<int>>> temporay_vector;
  temporay_vector.resize(m_new_vector_size);
  for (i = 0; i != m_new_vector_size; i++) {
    temporay_vector[i].resize(m_new_vector_size);
    for (j = 0; j != m_new_vector_size; j++) {
      temporay_vector[i][j].resize(m_new_vector_size);
      for (k = 0; k != m_new_vector_size; k++) {
        temporay_vector[i][j][k] = 2;
      }
    }
  }

  ///判断将上层网格点对应当前网格点像素值相反的网格点作为交换点，并且排除上层作为全零和全1的情况
  int sum_xy = 0;
  int sum_xz = 0;
  int sum_yz = 0;
  int sum_3D = 0;
  for (i = 0; i != m_new_vector_size; i++) {
    for (j = 0; j != m_new_vector_size; j++) {
      for (k = 0; k != m_new_vector_size; k++) {
        ///////////////////////////////////////////////////////////////////
        if (i % 2 == 1 && j % 2 == 1 && k % 2 == 1 &&
            (m_preproc_vector[i][j][k] ==
             2 /*||m_preproc_vector[i][j][k]==300*/))  //存放预处理结果容器，标注3*3全为黑或全白的区域
        {                                              // 190212
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + p + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
            }
          }
          if (m_reconstruct_vec[i][j][k] == 1 && sum_xy != 9 && sum_xz != 9 &&
              sum_yz != 9) {
            temporay_vector[i][j][k] = 1;
          } else if (m_reconstruct_vec[i][j][k] == 0 && sum_xy != 0 &&
                     sum_xz != 0 && sum_yz != 0) {
            temporay_vector[i][j][k] = 0;
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 1 && j % 2 == 1 && k % 2 == 0 &&
                 (m_preproc_vector[i][j][k] == 2)) {
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + q + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;  //变成固定点
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (sum_xy != 9 && (sum_xz != 9 || sum_yz != 9) &&
                m_reconstruct_vec[i][j][k] == 1) {
              temporay_vector[i][j][k] = 1;
            } else if (sum_xy != 0 && (sum_xz != 0 || sum_yz != 0) &&
                       m_reconstruct_vec[i][j][k] == 0) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 1 && j % 2 == 0 && k % 2 == 1 &&
                 (m_preproc_vector[i][j][k] == 2)) {
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + q + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (sum_xz != 9 && (sum_xy != 9 || sum_yz != 9) &&
                m_reconstruct_vec[i][j][k] == 1) {
              temporay_vector[i][j][k] = 1;
            } else if (sum_xz != 0 && (sum_xy != 0 || sum_yz != 0) &&
                       m_reconstruct_vec[i][j][k] == 0) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 0 && j % 2 == 1 && k % 2 == 1 &&
                 (m_preproc_vector[i][j][k] == 2)) {  // 190212
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + q + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (sum_yz != 9 && (sum_xy != 9 || sum_xz != 9) &&
                m_reconstruct_vec[i][j][k] == 1) {
              temporay_vector[i][j][k] = 1;
            } else if (sum_yz != 0 && (sum_xy != 0 || sum_xz != 0) &&
                       m_reconstruct_vec[i][j][k] == 0) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 0 && j % 2 == 0 && k % 2 == 1 &&
                 (m_preproc_vector[i][j][k] == 2)) {  // 190212
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + p + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (m_reconstruct_vec[i][j][k] == 1 &&
                (sum_xy != 9 || (sum_xz != 9 && sum_yz != 9))) {
              temporay_vector[i][j][k] = 1;
            } else if (m_reconstruct_vec[i][j][k] == 0 &&
                       (sum_xy != 0 || (sum_xz != 0 && sum_yz != 0))) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 0 && j % 2 == 1 && k % 2 == 0 &&
                 (m_preproc_vector[i][j][k] == 2)) {  // 190212
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + p + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (m_reconstruct_vec[i][j][k] == 1 &&
                (sum_xz != 9 || (sum_xy != 9 && sum_yz != 9))) {
              temporay_vector[i][j][k] = 1;
            } else if (m_reconstruct_vec[i][j][k] == 0 &&
                       (sum_xz != 0 || (sum_xy != 0 && sum_yz != 0))) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 1 && j % 2 == 0 && k % 2 == 0 &&
                 (m_preproc_vector[i][j][k] == 2)) {  // 190212
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + p + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (m_reconstruct_vec[i][j][k] == 1 &&
                (sum_yz != 9 || (sum_xy != 9 && sum_xz != 9))) {
              temporay_vector[i][j][k] = 1;
            } else if (m_reconstruct_vec[i][j][k] == 0 &&
                       (sum_yz != 0 || (sum_xy != 0 && sum_xz != 0))) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }
      }
    }
  }
  // temporay_vector存放选择交换点（190212：大孔孔点的位置不能参数交换）
  // Sign_exchange_vector存放交换成功与否情况
  // m_preproc_vector存放已经确定的点，不再参与交换

  //////////////////////////////////////////////////////////选取交换点
  white_point_number = 0;
  black_point_number = 0;
  bool judgement = false;

  for (i = 0; i != m_new_vector_size; i++) {
    for (j = 0; j != m_new_vector_size; j++) {
      for (k = 0; k != m_new_vector_size; k++) {
        if (temporay_vector[i][j][k] == 1) {
          m_white_posx.push_back(i);
          m_white_posy.push_back(j);
          m_white_posz.push_back(k);
          ++white_point_number;
        } else if (temporay_vector[i][j][k] == 0) {
          m_black_posx.push_back(i);
          m_black_posy.push_back(j);
          m_black_posz.push_back(k);
          ++black_point_number;
        }
      }
    }
  }

  m_class_white_point_number = white_point_number;
  m_class_black_point_number = black_point_number;
  temporay_vector.clear();
}
// temporay_vector存放选择交换点
void CAnnealing::Select_final_exchange_point_uncondition_2fuse3(
    int& white_point_number, int& black_point_number) {
  int i = 0, j = 0, k = 0, p = 0, q = 0, g = 0, n = 0;
  vector<vector<vector<int>>> temporay_vector;
  temporay_vector.resize(m_new_vector_size);
  for (i = 0; i != m_new_vector_size; i++) {
    temporay_vector[i].resize(m_new_vector_size);
    for (j = 0; j != m_new_vector_size; j++) {
      temporay_vector[i][j].resize(m_new_vector_size);
      for (k = 0; k != m_new_vector_size; k++) {
        temporay_vector[i][j][k] = 2;
      }
    }
  }

  ///判断将上层网格点对应当前网格点像素值相反的网格点作为交换点，并且排除上层作为全零和全1的情况
  int sum_xy = 0;
  int sum_xz = 0;
  int sum_yz = 0;
  int sum_3D = 0;
  for (i = 0; i != m_new_vector_size; i++) {
    for (j = 0; j != m_new_vector_size; j++) {
      for (k = 0; k != m_new_vector_size; k++) {
        ///////////////////////////////////////////////////////////////////
        if (i % 2 == 1 && j % 2 == 1 && k % 2 == 1 &&
            (m_preproc_vector[i][j][k] ==
             2 /*||m_preproc_vector[i][j][k]==300*/))  //存放预处理结果容器，标注3*3全为黑或全白的区域
        {                                              // 190212
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + p + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
            }
          }
          if (m_reconstruct_vec[i][j][k] == 1 && sum_xy != 9 && sum_xz != 9 &&
              sum_yz != 9) {
            temporay_vector[i][j][k] = 1;
          } else if (m_reconstruct_vec[i][j][k] == 0 && sum_xy != 0 &&
                     sum_xz != 0 && sum_yz != 0) {
            temporay_vector[i][j][k] = 0;
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 1 && j % 2 == 1 && k % 2 == 0 &&
                 (m_preproc_vector[i][j][k] == 2 ||
                  m_preproc_vector[i][j][k] == 300)) {  // 190212
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + q + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            if (m_preproc_vector[i][j][k] != 300)  // 190212
            {
              m_preproc_vector[i][j][k] = 1;  //变成固定点
            }
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (sum_xy != 9 && (sum_xz != 9 || sum_yz != 9) &&
                m_reconstruct_vec[i][j][k] == 1) {
              if (m_preproc_vector[i][j][k] != 300)  // 190212
              {
                temporay_vector[i][j][k] = 1;
              }
            } else if (sum_xy != 0 && (sum_xz != 0 || sum_yz != 0) &&
                       m_reconstruct_vec[i][j][k] == 0) {
              if (m_preproc_vector[i][j][k] !=
                  300)  // 190212：大孔孔点位置不允许交换（只能为小孔的背景点）
                        // //光看这一句会觉得：有些300变成20/30了呀？但大前提if限定了pre为2/300
              {
                temporay_vector[i][j][k] = 0;
              }
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 1 && j % 2 == 0 && k % 2 == 1 &&
                 (m_preproc_vector[i][j][k] == 2 ||
                  m_preproc_vector[i][j][k] == 300)) {  // 190212
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + q + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            if (m_preproc_vector[i][j][k] != 300)  // 190212
            {
              m_preproc_vector[i][j][k] = 1;
            }
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (sum_xz != 9 && (sum_xy != 9 || sum_yz != 9) &&
                m_reconstruct_vec[i][j][k] == 1) {
              if (m_preproc_vector[i][j][k] != 300)  // 190212
              {
                temporay_vector[i][j][k] = 1;
              }
            } else if (sum_xz != 0 && (sum_xy != 0 || sum_yz != 0) &&
                       m_reconstruct_vec[i][j][k] == 0) {
              if (m_preproc_vector[i][j][k] !=
                  300)  // 190212：大孔孔点位置不允许交换（只能为小孔的背景点）
              {
                temporay_vector[i][j][k] = 0;
              }
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 0 && j % 2 == 1 && k % 2 == 1 &&
                 (m_preproc_vector[i][j][k] == 2 ||
                  m_preproc_vector[i][j][k] == 300)) {  // 190212
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + q + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            if (m_preproc_vector[i][j][k] != 300)  // 190212
            {
              m_preproc_vector[i][j][k] = 1;
            }
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (sum_yz != 9 && (sum_xy != 9 || sum_xz != 9) &&
                m_reconstruct_vec[i][j][k] == 1) {
              if (m_preproc_vector[i][j][k] != 300)  // 190212
              {
                temporay_vector[i][j][k] = 1;
              }
            } else if (sum_yz != 0 && (sum_xy != 0 || sum_xz != 0) &&
                       m_reconstruct_vec[i][j][k] == 0) {
              if (m_preproc_vector[i][j][k] !=
                  300)  // 190212：大孔孔点位置不允许交换（只能为小孔的背景点）
              {
                temporay_vector[i][j][k] = 0;
              }
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 0 && j % 2 == 0 && k % 2 == 1 &&
                 (m_preproc_vector[i][j][k] == 2 ||
                  m_preproc_vector[i][j][k] == 300)) {  // 190212
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + p + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            if (m_preproc_vector[i][j][k] != 300)  // 190212
            {
              m_preproc_vector[i][j][k] = 1;
            }
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (m_reconstruct_vec[i][j][k] == 1 &&
                (sum_xy != 9 || (sum_xz != 9 && sum_yz != 9))) {
              if (m_preproc_vector[i][j][k] != 300)  // 190212
              {
                temporay_vector[i][j][k] = 1;
              }
            } else if (m_reconstruct_vec[i][j][k] == 0 &&
                       (sum_xy != 0 || (sum_xz != 0 && sum_yz != 0))) {
              if (m_preproc_vector[i][j][k] !=
                  300)  // 190212：大孔孔点位置不允许交换（只能为小孔的背景点）
              {
                temporay_vector[i][j][k] = 0;
              }
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 0 && j % 2 == 1 && k % 2 == 0 &&
                 (m_preproc_vector[i][j][k] == 2 ||
                  m_preproc_vector[i][j][k] == 300)) {  // 190212
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + p + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            if (m_preproc_vector[i][j][k] != 300)  // 190212
            {
              m_preproc_vector[i][j][k] = 1;
            }
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (m_reconstruct_vec[i][j][k] == 1 &&
                (sum_xz != 9 || (sum_xy != 9 && sum_yz != 9))) {
              if (m_preproc_vector[i][j][k] != 300)  // 190212
              {
                temporay_vector[i][j][k] = 1;
              }
            } else if (m_reconstruct_vec[i][j][k] == 0 &&
                       (sum_xz != 0 || (sum_xy != 0 && sum_yz != 0))) {
              if (m_preproc_vector[i][j][k] !=
                  300)  // 190212：大孔孔点位置不允许交换（只能为小孔的背景点）
              {
                temporay_vector[i][j][k] = 0;
              }
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 1 && j % 2 == 0 && k % 2 == 0 &&
                 (m_preproc_vector[i][j][k] == 2 ||
                  m_preproc_vector[i][j][k] == 300)) {  // 190212
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy = sum_xy + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size]
                                                 [(j + p + m_new_vector_size) %
                                                  m_new_vector_size][k];
              sum_xz = sum_xz + m_reconstruct_vec[(i + q + m_new_vector_size) %
                                                  m_new_vector_size][j]
                                                 [(k + p + m_new_vector_size) %
                                                  m_new_vector_size];
              sum_yz = sum_yz +
                       m_reconstruct_vec[i][(j + p + m_new_vector_size) %
                                            m_new_vector_size]
                                        [(k + p + m_new_vector_size) %
                                         m_new_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D = sum_3D +
                         m_reconstruct_vec
                             [(i + q + m_new_vector_size) % m_new_vector_size]
                             [(j + p + m_new_vector_size) % m_new_vector_size]
                             [(k + g + m_new_vector_size) % m_new_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            if (m_preproc_vector[i][j][k] != 300)  // 190212
            {
              m_preproc_vector[i][j][k] = 1;
            }
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (m_reconstruct_vec[i][j][k] == 1 &&
                (sum_yz != 9 || (sum_xy != 9 && sum_xz != 9))) {
              if (m_preproc_vector[i][j][k] != 300)  // 190212
              {
                temporay_vector[i][j][k] = 1;
              }
            } else if (m_reconstruct_vec[i][j][k] == 0 &&
                       (sum_yz != 0 || (sum_xy != 0 && sum_xz != 0))) {
              if (m_preproc_vector[i][j][k] !=
                  300)  // 190212：大孔孔点位置不允许交换（只能为小孔的背景点）
              {
                temporay_vector[i][j][k] = 0;
              }
            }
          }
        }
      }
    }
  }
  // temporay_vector存放选择交换点（190212：大孔孔点的位置不能参数交换）
  // Sign_exchange_vector存放交换成功与否情况
  // m_preproc_vector存放已经确定的点，不再参与交换

  //////////////////////////////////////////////////////////选取交换点
  white_point_number = 0;
  black_point_number = 0;
  bool judgement = false;

  for (i = 0; i != m_new_vector_size; i++) {
    for (j = 0; j != m_new_vector_size; j++) {
      for (k = 0; k != m_new_vector_size; k++) {
        if (temporay_vector[i][j][k] == 1) {
          m_white_posx.push_back(i);
          m_white_posy.push_back(j);
          m_white_posz.push_back(k);
          ++white_point_number;
        } else if (temporay_vector[i][j][k] == 0) {
          m_black_posx.push_back(i);
          m_black_posy.push_back(j);
          m_black_posz.push_back(k);
          ++black_point_number;
        }
      }
    }
  }

  m_class_white_point_number = white_point_number;
  m_class_black_point_number = black_point_number;
  temporay_vector.clear();
}

//随机交换的原理？？
void CAnnealing::Random_position(int loop) {
  int rand_data_1 = 0;
  int rand_data_2 = 0;
  int temperory = 0;
  int rand_number_white = (1.8 + 0.3 * (loop % 2)) * m_class_white_point_number;
  int rand_number_black = (1.8 + 0.2 * (loop % 4)) * m_class_black_point_number;
  while (rand_number_white != 0) {
    rand_data_1 = rand32() % m_class_white_point_number;
    rand_data_2 = rand32() % m_class_white_point_number;
    if (rand_data_2 != rand_data_1) {
      temperory = m_white_posx[rand_data_1];
      m_white_posx[rand_data_1] = m_white_posx[rand_data_2];
      m_white_posx[rand_data_2] = temperory;
      temperory = m_white_posy[rand_data_1];
      m_white_posy[rand_data_1] = m_white_posy[rand_data_2];
      m_white_posy[rand_data_2] = temperory;
      temperory = m_white_posz[rand_data_1];
      m_white_posz[rand_data_1] = m_white_posz[rand_data_2];
      m_white_posz[rand_data_2] = temperory;
      rand_number_white--;
    }
  }
  while (rand_number_black != 0) {
    rand_data_1 = rand32() % m_class_black_point_number;
    rand_data_2 = rand32() % m_class_black_point_number;
    if (rand_data_2 != rand_data_1) {
      temperory = m_black_posx[rand_data_1];
      m_black_posx[rand_data_1] = m_black_posx[rand_data_2];
      m_black_posx[rand_data_2] = temperory;
      temperory = m_black_posy[rand_data_1];
      m_black_posy[rand_data_1] = m_black_posy[rand_data_2];
      m_black_posy[rand_data_2] = temperory;
      temperory = m_black_posz[rand_data_1];
      m_black_posz[rand_data_1] = m_black_posz[rand_data_2];
      m_black_posz[rand_data_2] = temperory;
      rand_number_black--;
    }
  }
}

// white/black_rand_site 白/黑色交换点数位置
void CAnnealing::Exchange_two_point() {
  m_white_randsite = m_class_white_point_number - 1;
  m_black_randsite = m_class_black_point_number - 1;
  // white_rand_site=rand32()%class_white_point_number;
  // black_rand_site=rand32()%class_black_point_number;
}

void CAnnealing::Get_exchange_site() {
  m_white_site_x = m_white_posx.begin() + m_white_randsite;
  m_white_site_y = m_white_posy.begin() + m_white_randsite;
  m_white_site_z = m_white_posz.begin() + m_white_randsite;
  m_black_site_x = m_black_posx.begin() + m_black_randsite;
  m_black_site_y = m_black_posy.begin() + m_black_randsite;
  m_black_site_z = m_black_posz.begin() + m_black_randsite;
}

void CAnnealing::Recstruct_Multi_point_density_function_fast_p(
    int& Engerry_down, int& Engerry_change_white, int& Engerry_change_black) {
  Engerry_change_white = 0;
  Engerry_change_black = 0;
  int i = 0, j = 0;
  int code_value = 0;  //模式编码值
  int q = 0, p = 0;
  int k = m_Rec_template_size / 2;
  int white_x = m_white_posx[m_white_randsite];
  int white_y = m_white_posy[m_white_randsite];
  int white_z = m_white_posz[m_white_randsite];
  int black_x = m_black_posx[m_black_randsite];
  int black_y = m_black_posy[m_black_randsite];
  int black_z = m_black_posz[m_black_randsite];
  pair<map<int, int>::iterator, bool> pair_code_value;

  //这里首先将Exchange_code_value_white/black清空了，这样在执行下面的--操作时，为什么不会出错？
  m_exchange_code_value_white.clear();
  m_exchange_code_value_black.clear();
  for (q = -k; q != k + 1; q++)  ////次循环为每个交换点决定的9个模式
  {
    for (p = -k; p != k + 1; p++) {
      m_reconstruct_vec[white_x][white_y][white_z] =
          1;  //交换前(190212：坐标是由temporay_vector存放选择的交换点确定的)
      m_reconstruct_vec[black_x][black_y][black_z] = 0;

      ////////////////////////////////////////////////////XY平面交换点决定的模式

      code_value = 0;
      for (i = -k; i != k + 1; i++)  ////此循环为计算每个模式
      {
        for (j = -k; j != k + 1; j++) {
          code_value = code_value * 2 +
                       m_reconstruct_vec[(i + white_x + q + m_new_vector_size) %
                                         m_new_vector_size]
                                        [(j + white_y + p + m_new_vector_size) %
                                         m_new_vector_size][white_z];
        }
      }
      --m_exchange_code_value_white
          [code_value];  //减去交换前由白色交换点确定的模式

      if (white_z != black_z) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[(i + black_x + q + m_new_vector_size) %
                                  m_new_vector_size]
                                 [(j + black_y + p + m_new_vector_size) %
                                  m_new_vector_size][black_z];
          }
        }
        --m_exchange_code_value_black
            [code_value];  //减去交换前由黑色交换点确定的模式
      }

      else if (abs((black_x + q + m_new_vector_size) % m_new_vector_size -
                   white_x) > k ||
               abs((black_y + p + m_new_vector_size) % m_new_vector_size -
                   white_y) > k) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[(i + black_x + q + m_new_vector_size) %
                                  m_new_vector_size]
                                 [(j + black_y + p + m_new_vector_size) %
                                  m_new_vector_size][black_z];
          }
        }
        --m_exchange_code_value_black
            [code_value];  //减去交换前由黑色交换点确定的模式
      }

      ////////////////////////////////////////////////////XZ平面交换点决定的模式
      code_value = 0;
      for (i = -k; i != k + 1; i++) {
        for (j = -k; j != k + 1; j++) {
          code_value = code_value * 2 +
                       m_reconstruct_vec[(i + white_x + q + m_new_vector_size) %
                                         m_new_vector_size][white_y]
                                        [(j + white_z + p + m_new_vector_size) %
                                         m_new_vector_size];
        }
      }
      --m_exchange_code_value_white
          [code_value];  //减去交换前由白色交换点确定的模式

      if (white_y != black_y) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[(i + black_x + q + m_new_vector_size) %
                                  m_new_vector_size][black_y]
                                 [(j + black_z + p + m_new_vector_size) %
                                  m_new_vector_size];
          }
        }
        --m_exchange_code_value_black
            [code_value];  //减去交换前由黑色交换点确定的模式
      }

      else if (abs((black_x + q + m_new_vector_size) % m_new_vector_size -
                   white_x) > k ||
               abs((black_z + p + m_new_vector_size) % m_new_vector_size -
                   white_z) > k) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[(i + black_x + q + m_new_vector_size) %
                                  m_new_vector_size][black_y]
                                 [(j + black_z + p + m_new_vector_size) %
                                  m_new_vector_size];
          }
        }
        --m_exchange_code_value_black
            [code_value];  //减去交换前由黑色交换点确定的模式
      }

      //////////////////////////////////////////////////YZ平面交换点决定的模式
      code_value = 0;
      for (i = -k; i != k + 1; i++) {
        for (j = -k; j != k + 1; j++) {
          code_value =
              code_value * 2 +
              m_reconstruct_vec[white_x][(i + white_y + q + m_new_vector_size) %
                                         m_new_vector_size]
                               [(j + white_z + p + m_new_vector_size) %
                                m_new_vector_size];
        }
      }
      --m_exchange_code_value_white
          [code_value];  //减去交换前由白色交换点确定的模式

      if (white_x != black_x) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[black_x]
                                 [(i + black_y + q + m_new_vector_size) %
                                  m_new_vector_size]
                                 [(j + black_z + p + m_new_vector_size) %
                                  m_new_vector_size];
          }
        }
        --m_exchange_code_value_black
            [code_value];  //减去交换前由黑色交换点确定的模式
      } else if (abs((black_y + q + m_new_vector_size) % m_new_vector_size -
                     white_y) > k ||
                 abs((black_z + p + m_new_vector_size) % m_new_vector_size -
                     white_z) > k) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[black_x]
                                 [(i + black_y + q + m_new_vector_size) %
                                  m_new_vector_size]
                                 [(j + black_z + p + m_new_vector_size) %
                                  m_new_vector_size];
          }
        }
        --m_exchange_code_value_black
            [code_value];  //减去交换前由黑色交换点确定的模式
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      m_reconstruct_vec[white_x][white_y][white_z] = 0;  //交换后
      m_reconstruct_vec[black_x][black_y][black_z] = 1;
      ////////////////////////////////////////////////////XY平面交换点决定的模式
      code_value = 0;
      for (i = -k; i != k + 1; ++i) {
        for (j = -k; j != k + 1; ++j) {
          code_value = code_value * 2 +
                       m_reconstruct_vec[(i + white_x + q + m_new_vector_size) %
                                         m_new_vector_size]
                                        [(j + white_y + p + m_new_vector_size) %
                                         m_new_vector_size][white_z];
        }
      }
      pair_code_value = m_exchange_code_value_white.insert(
          make_pair(code_value, 1));  //加上交换前由白色交换点确定的模式
      if (!pair_code_value.second) {
        ++pair_code_value.first->second;
      }

      if (white_z != black_z) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[(i + black_x + q + m_new_vector_size) %
                                  m_new_vector_size]
                                 [(j + black_y + p + m_new_vector_size) %
                                  m_new_vector_size][black_z];
          }
        }
        pair_code_value =
            m_exchange_code_value_black.insert(make_pair(code_value, 1));
        if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
        {
          ++pair_code_value.first->second;
        }
      } else if (abs((black_x + q + m_new_vector_size) % m_new_vector_size -
                     white_x) > k ||
                 abs((black_y + p + m_new_vector_size) % m_new_vector_size -
                     white_y) > k) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[(i + black_x + q + m_new_vector_size) %
                                  m_new_vector_size]
                                 [(j + black_y + p + m_new_vector_size) %
                                  m_new_vector_size][black_z];
          }
        }
        pair_code_value =
            m_exchange_code_value_black.insert(make_pair(code_value, 1));
        if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
        {
          ++pair_code_value.first->second;
        }
      }

      ////////////////////////////////////////////////////YZ平面交换点决定的模式
      code_value = 0;
      for (i = -k; i != k + 1; ++i) {
        for (j = -k; j != k + 1; ++j) {
          code_value =
              code_value * 2 +
              m_reconstruct_vec[white_x][(j + white_y + p + m_new_vector_size) %
                                         m_new_vector_size]
                               [(i + white_z + q + m_new_vector_size) %
                                m_new_vector_size];
        }
      }
      pair_code_value = m_exchange_code_value_white.insert(
          make_pair(code_value, 1));  //加上交换前由白色交换点确定的模式
      if (!pair_code_value.second) {
        ++pair_code_value.first->second;
      }

      if (white_x != black_x) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[black_x]
                                 [(j + black_y + p + m_new_vector_size) %
                                  m_new_vector_size]
                                 [(i + black_z + q + m_new_vector_size) %
                                  m_new_vector_size];
          }
        }
        pair_code_value =
            m_exchange_code_value_black.insert(make_pair(code_value, 1));
        if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
        {
          ++pair_code_value.first->second;
        }
      } else if (abs((black_y + q + m_new_vector_size) % m_new_vector_size -
                     white_y) > k ||
                 abs((black_z + p + m_new_vector_size) % m_new_vector_size -
                     white_z) > k) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[black_x]
                                 [(j + black_y + p + m_new_vector_size) %
                                  m_new_vector_size]
                                 [(i + black_z + q + m_new_vector_size) %
                                  m_new_vector_size];
          }
        }
        pair_code_value =
            m_exchange_code_value_black.insert(make_pair(code_value, 1));
        if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
        {
          ++pair_code_value.first->second;
        }
      }

      ////////////////////////////////////////////////////XZ平面交换点决定的模式
      code_value = 0;
      for (i = -k; i != k + 1; ++i) {
        for (j = -k; j != k + 1; ++j) {
          code_value = code_value * 2 +
                       m_reconstruct_vec[(i + white_x + q + m_new_vector_size) %
                                         m_new_vector_size][white_y]
                                        [(j + white_z + p + m_new_vector_size) %
                                         m_new_vector_size];
        }
      }
      pair_code_value = m_exchange_code_value_white.insert(
          make_pair(code_value, 1));  //加上交换前由白色交换点确定的模式
      if (!pair_code_value.second) {
        ++pair_code_value.first->second;
      }

      if (white_y != black_y) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[(i + black_x + q + m_new_vector_size) %
                                  m_new_vector_size][black_y]
                                 [(j + black_z + p + m_new_vector_size) %
                                  m_new_vector_size];
          }
        }
        pair_code_value =
            m_exchange_code_value_black.insert(make_pair(code_value, 1));
        if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
        {
          ++pair_code_value.first->second;
        }
      } else if (abs((black_x + q + m_new_vector_size) % m_new_vector_size -
                     white_x) > k ||
                 abs((black_z + p + m_new_vector_size) % m_new_vector_size -
                     white_z) > k) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[(i + black_x + q + m_new_vector_size) %
                                  m_new_vector_size][black_y]
                                 [(j + black_z + p + m_new_vector_size) %
                                  m_new_vector_size];
          }
        }
        pair_code_value =
            m_exchange_code_value_black.insert(make_pair(code_value, 1));
        if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
        {
          ++pair_code_value.first->second;
        }
      }
    }
  }
  int Engerry_before_exchange_white = 0;
  int Engerry_after_exchange_white = 0;
  int Engerry_before_exchange_black = 0;
  int Engerry_after_exchange_black = 0;
  map<int, int>::iterator iter_Success = m_exchange_code_value_white.begin();
  for (iter_Success; iter_Success != m_exchange_code_value_white.end();
       ++iter_Success) {
    // 190113
    Engerry_before_exchange_white =
        Engerry_before_exchange_white +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_new_vector_size * m_train_code_value[iter_Success->first] *
                (m_multiple * m_multiple));  //计算没有变化的模式个数
    m_reconstruct_code_value[iter_Success->first] =
        m_reconstruct_code_value[iter_Success->first] +
        m_exchange_code_value_white[iter_Success->first];  //保存原来的模式
    Engerry_after_exchange_white =
        Engerry_after_exchange_white +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_new_vector_size * m_train_code_value[iter_Success->first] *
                (m_multiple * m_multiple));  //计算模式差值
  }
  Engerry_change_white =
      Engerry_after_exchange_white - Engerry_before_exchange_white;

  iter_Success = m_exchange_code_value_black.begin();
  for (iter_Success; iter_Success != m_exchange_code_value_black.end();
       ++iter_Success) {
    Engerry_before_exchange_black =
        Engerry_before_exchange_black +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_new_vector_size * m_train_code_value[iter_Success->first] *
                (m_multiple * m_multiple));  //计算没有变化的模式个数
    m_reconstruct_code_value[iter_Success->first] =
        m_reconstruct_code_value[iter_Success->first] +
        m_exchange_code_value_black[iter_Success->first];  //保存原来的模式
    Engerry_after_exchange_black =
        Engerry_after_exchange_black +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_new_vector_size * m_train_code_value[iter_Success->first] *
                (m_multiple * m_multiple));  //计算模式差值
  }
  Engerry_change_black =
      Engerry_after_exchange_black - Engerry_before_exchange_black;

  m_Enerry_Reverse = m_Enerry_origial;  //将交换前的能量差保存，以便替换回来
  Engerry_down = m_Enerry_origial + Engerry_change_white +
                 Engerry_change_black;  //传递交换后的能量差
  m_Enerry_origial = Engerry_down;      //将当期能量差作为新状态
  m_reconstruct_vec[white_x][white_y][white_z] = 0;  //交换后
  m_reconstruct_vec[black_x][black_y][black_z] = 1;
}

void CAnnealing::Delete_exchange_point() {
  // white_point_x.erase(white_site_x);
  // white_point_y.erase(white_site_y);
  // white_point_z.erase(white_site_z);
  // black_point_x.erase(black_site_x);
  // black_point_y.erase(black_site_y);
  // black_point_z.erase(black_site_z);
  m_class_white_point_number--;
  m_class_black_point_number--;
}

void CAnnealing::Recover_Multi_point_density_function() {
  map<int, int>::iterator iter_F = m_exchange_code_value_black.begin();
  for (iter_F; iter_F != m_exchange_code_value_black.end(); ++iter_F) {
    m_reconstruct_code_value[iter_F->first] =
        m_reconstruct_code_value[iter_F->first] -
        m_exchange_code_value_black[iter_F->first];
  }
  iter_F = m_exchange_code_value_white.begin();
  for (iter_F; iter_F != m_exchange_code_value_white.end(); ++iter_F) {
    m_reconstruct_code_value[iter_F->first] =
        m_reconstruct_code_value[iter_F->first] -
        m_exchange_code_value_white[iter_F->first];
  }
  m_Enerry_origial = m_Enerry_Reverse;
}

void CAnnealing::Reverse_exchange_point() {
  m_reconstruct_vec[m_white_posx[m_white_randsite]]
                   [m_white_posy[m_white_randsite]]
                   [m_white_posz[m_white_randsite]] = 1;
  m_reconstruct_vec[m_black_posx[m_black_randsite]]
                   [m_black_posy[m_black_randsite]]
                   [m_black_posz[m_black_randsite]] = 0;
}

void CAnnealing::Delete_unexchange_white_point() {
  // white_point_x.erase(white_site_x);
  // white_point_y.erase(white_site_y);
  // white_point_z.erase(white_site_z);
  m_class_white_point_number--;
}

void CAnnealing::Delete_unexchange_black_point() {
  // black_point_x.erase(black_site_x);
  // black_point_y.erase(black_site_y);
  // black_point_z.erase(black_site_z);
  m_class_black_point_number--;
}

void CAnnealing::Delete_unexchange_two_phase_point() {
  // white_point_x.erase(white_site_x);
  // white_point_y.erase(white_site_y);
  // white_point_z.erase(white_site_z);
  // black_point_x.erase(black_site_x);
  // black_point_y.erase(black_site_y);
  // black_point_z.erase(black_site_z);
  m_class_white_point_number--;
  m_class_black_point_number--;
}

void CAnnealing::Delete_site_vector() {
  m_white_posx.clear();
  m_white_posy.clear();
  m_white_posz.clear();
  m_black_posx.clear();
  m_black_posy.clear();
  m_black_posz.clear();
  m_class_white_point_number = 0;
  m_class_black_point_number = 0;
}

void CAnnealing::Select_inital_exchange_point_uncondition_Recons(
    int& white_point_number, int& black_point_number) {
  int i = 0, j = 0, k = 0;
  int p = 0, q = 0, g = 0;
  white_point_number = 0;
  black_point_number = 0;

  for (i = 0; i != m_new_vector_size; i++) {
    for (j = 0; j != m_new_vector_size; j++) {
      for (k = 0; k != m_new_vector_size; k++) {
        if (m_reconstruct_vec[i][j][k] == 1)  //存放重建结果容器
        {
          m_white_posx.push_back(i);
          m_white_posy.push_back(j);
          m_white_posz.push_back(k);
          ++white_point_number;
        } else if (m_reconstruct_vec[i][j][k] == 0) {
          m_black_posx.push_back(i);
          m_black_posy.push_back(j);
          m_black_posz.push_back(k);
          ++black_point_number;
        }
      }
    }
  }
  m_class_white_point_number = white_point_number;
  m_class_black_point_number = black_point_number;
}

void CAnnealing::Set_reconstruct_to_inital() {
  int i = 0, j = 0, k = 0;
  m_initial_rand_data.resize(m_new_vector_size);  // 190113
  for (i = 0; i != m_new_vector_size; ++i) {
    m_initial_rand_data[i].resize(m_new_vector_size);
    for (j = 0; j != m_new_vector_size; ++j) {
      m_initial_rand_data[i][j].resize(m_new_vector_size);
    }
  }
  for (i = 0; i != m_new_vector_size; ++i) {
    for (j = 0; j != m_new_vector_size; ++j) {
      for (k = 0; k != m_new_vector_size; k++) {
        m_initial_rand_data[i][j][k] = m_reconstruct_vec[i][j][k];
      }
    }
  }
}

void CAnnealing::Putout_step_image(const QString& filepath) {
  QDir qdir(filepath);
  if (!qdir.exists("reconstruct")) qdir.mkdir("reconstruct");
  QImage img(m_vector_size, m_vector_size, QImage::Format_Grayscale8);
  int i = 0, j = 0, k = 0, q = 0, white_point = 0;
  for (q = 0; q != m_new_vector_size; q++) {
    for (i = 0; i != m_new_vector_size; ++i) {
      uchar* lpix = img.scanLine(i);
      for (j = 0; j != m_new_vector_size; ++j) {
        k = m_reconstruct_vec[q][i][j];
        *(lpix + j * 1) = (k ? 255 : 0);
      }
    }
    char name[100];
    sprintf(name, "%d.bmp", q);
    //最终图片名格式为数字编号，例“1.bmp”
    QString respath = filepath;
    respath.append("/reconstruct/").append(name);
    img.save(respath, nullptr, 100);
  }
}

// 190213：保存包含大孔的三维孔隙结构
void CAnnealing::Set_reconstruct_to_final() {
  int i = 0, j = 0, k = 0;
  if (m_final_imgsize != m_new_vector_size) {
    cout << "重建尺寸出错！" << endl;
    return;
  }

  m_initial_rand_data.resize(m_final_imgsize);  // 190113
  for (i = 0; i != m_final_imgsize; ++i) {
    m_initial_rand_data[i].resize(m_final_imgsize);
    for (j = 0; j != m_final_imgsize; ++j) {
      m_initial_rand_data[i][j].resize(m_final_imgsize);
    }
  }
  for (i = 0; i != m_final_imgsize; ++i) {
    for (j = 0; j != m_final_imgsize; ++j) {
      for (k = 0; k != m_final_imgsize; k++) {
        if (m_big_3Dimgvec[i][j][k] != 0 || m_reconstruct_vec[i][j][k] != 0) {
          m_initial_rand_data[i][j][k] = 255;
        } else {
          m_initial_rand_data[i][j][k] = 0;
        }
      }
    }
  }

  // Putout_final_image();
}

void CAnnealing::Putout_final_image(const QString& filepath) {
  QDir qdir(filepath);
  if (!qdir.exists("fuse")) qdir.mkdir("fuse");
  QImage img(m_vector_size, m_vector_size, QImage::Format_Grayscale8);

  for (int i = 0; i < m_final_imgsize; i++) {
    for (int j = 0; j < m_final_imgsize; j++) {
      uchar* lpix = img.scanLine(j);
      for (int k = 0; k < m_final_imgsize; k++) {
        *(lpix + k * 1) = m_initial_rand_data[i][j][k];
      }
    }
    char name[100];
    sprintf(name, "%d.bmp", i);
    QString respath = filepath;
    respath.append("/fuse/").append(name);
    img.save(respath, nullptr, 100);
  }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void CAnnealing::TI_Multi_point_density_function() {
  m_train_code_value.clear();  //将上一网格存储的多点密度函数清空
  int i = 0, j = 0;
  int reduce_size = m_initial_height / m_vector_size;  //缩小倍数
  int k = m_Rec_template_size / 2;
  vector<vector<int>> Temporary_vector;
  Temporary_vector.resize(m_vector_size);
  for (i = 0; i != m_vector_size; ++i) {
    Temporary_vector[i].resize(m_vector_size);
  }

  //将对应点的原始图像放到中间过程容器
  for (i = 0; i != m_vector_size; i++) {
    for (j = 0; j != m_vector_size; j++) {
      if (m_initial_imgvec[i * reduce_size][j * reduce_size] == 1) {
        Temporary_vector[i][j] = 1;  //为了后面编码方便将255值变为1
      } else {
        Temporary_vector[i][j] = 0;
      }
    }
  }

  //计算此时缩小图像的多点密度函数
  int code_value = 0;  //模式编码值
  int q = 0, p = 0;
  for (i = 0; i != m_vector_size; ++i) {
    for (j = 0; j != m_vector_size; ++j) {
      code_value = 0;
      for (p = -k; p != k + 1; p++) {
        for (q = -k; q != k + 1; q++) {
          if (i + p >= 0 && i + p < m_vector_size && j + q >= 0 &&
              j + q < m_vector_size) {
            code_value = code_value * 2 + Temporary_vector[i + p][j + q];
          }
        }
      }
      pair<map<int, int>::iterator, bool> pair_code_value =
          m_train_code_value.insert(make_pair(code_value, 1));
      if (!pair_code_value.second) {
        ++pair_code_value.first->second;
      }
    }
  }
}

void CAnnealing::Recstruct_Multi_point_density_function() {
  m_reconstruct_code_value.clear();  //将上一网格存储的多点密度函数清空
  int i = 0, j = 0, k = 0, q = 0, p = 0, r = 0;
  int s = m_Rec_template_size / 2;
  pair<map<int, int>::iterator, bool>
      pair_code_value;  //计算此时重建图像的多点密度函数
  int code_value = 0;   //模式编码值
  for (k = 0; k != m_vector_size; ++k) {
    for (j = 0; j != m_vector_size; ++j) {
      for (i = 0; i != m_vector_size; ++i) {
        code_value = 0;
        for (p = -s; p != s + 1; p++) {
          for (q = -s; q != s + 1; q++) {
            if (i + p >= 0 && i + p < m_vector_size && j + q >= 0 &&
                j + q < m_vector_size) {
              code_value = code_value * 2 + m_reconstruct_vec[k][i + p][j + q];
            }
          }
        }
        pair_code_value =
            m_reconstruct_code_value.insert(make_pair(code_value, 1));
        if (!pair_code_value.second) {
          ++pair_code_value.first->second;
        }
        code_value = 0;
        for (p = -s; p != s + 1; p++) {
          for (q = -s; q != s + 1; q++) {
            if (i + p >= 0 && i + p < m_vector_size && j + q >= 0 &&
                j + q < m_vector_size) {
              code_value = code_value * 2 + m_reconstruct_vec[i + p][k][j + q];
            }
          }
        }
        pair_code_value =
            m_reconstruct_code_value.insert(make_pair(code_value, 1));
        if (!pair_code_value.second) {
          ++pair_code_value.first->second;
        }
        code_value = 0;
        for (p = -s; p != s + 1; p++) {
          for (q = -s; q != s + 1; q++) {
            if (i + p >= 0 && i + p < m_vector_size && j + q >= 0 &&
                j + q < m_vector_size) {
              code_value = code_value * 2 + m_reconstruct_vec[i + p][j + q][k];
            }
          }
        }
        pair_code_value =
            m_reconstruct_code_value.insert(make_pair(code_value, 1));
        if (!pair_code_value.second) {
          ++pair_code_value.first->second;
        }
      }
    }
  }
}

void CAnnealing::Recstruct_Multi_point_density_function_fast(
    int& Engerry_down, int& Engerry_change_white, int& Engerry_change_black) {
  Engerry_change_white = 0;
  Engerry_change_black = 0;
  int i = 0, j = 0;
  int code_value = 0;  //模式编码值
  int q = 0, p = 0;
  int k = m_Rec_template_size / 2;
  int white_x = m_white_posx[m_white_randsite];
  int white_y = m_white_posy[m_white_randsite];
  int white_z = m_white_posz[m_white_randsite];
  int black_x = m_black_posx[m_black_randsite];
  int black_y = m_black_posy[m_black_randsite];
  int black_z = m_black_posz[m_black_randsite];
  pair<map<int, int>::iterator, bool> pair_code_value;
  m_exchange_code_value_white.clear();
  m_exchange_code_value_black.clear();
  for (q = -k; q != k + 1; q++) {
    for (p = -k; p != k + 1; p++) {
      m_reconstruct_vec[white_x][white_y][white_z] = 1;  //交换前
      m_reconstruct_vec[black_x][black_y][black_z] = 0;
      ////////////////////////////////////////////////////XY平面交换点决定的模式
      if ((white_x + q >= k) && (white_x + q < m_vector_size - k) &&
          (white_y + p >= k) && (white_y + p < m_vector_size - k)) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[i + white_x + q][j + white_y + p][white_z];
          }
        }
        --m_exchange_code_value_white
            [code_value];  //减去交换前由白色交换点确定的模式
      }
      if ((black_x + q >= k) && (q + black_x < m_vector_size - k) &&
          (black_y + p >= k) && (black_y + p < m_vector_size - k)) {
        if (white_z != black_z) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[i + black_x + q][j + black_y + p][black_z];
            }
          }
          --m_exchange_code_value_black
              [code_value];  //减去交换前由黑色交换点确定的模式
        } else if (((black_x + q) > (white_x + k)) ||
                   ((black_x + q) < (white_x - k)) ||
                   ((black_y + p) > (white_y + k)) ||
                   ((black_y + p) < (white_y - k))) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[i + black_x + q][j + black_y + p][black_z];
            }
          }
          --m_exchange_code_value_black
              [code_value];  //减去交换前由黑色交换点确定的模式
        }
      }
      ////////////////////////////////////////////////////XZ平面交换点决定的模式
      if ((white_x + q >= k) && (white_x + q < m_vector_size - k) &&
          (white_z + p >= k) && (white_z + p < m_vector_size - k)) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[i + white_x + q][white_y][j + white_z + p];
          }
        }
        --m_exchange_code_value_white
            [code_value];  //减去交换前由白色交换点确定的模式
      }
      if ((black_x + q >= k) && (q + black_x < m_vector_size - k) &&
          (black_z + p >= k) && (black_z + p < m_vector_size - k)) {
        if (white_y != black_y) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[i + black_x + q][black_y][j + black_z + p];
            }
          }
          --m_exchange_code_value_black
              [code_value];  //减去交换前由黑色交换点确定的模式
        } else if (((black_x + q) > (white_x + k)) ||
                   ((black_x + q) < (white_x - k)) ||
                   ((black_z + p) > (white_z + k)) ||
                   ((black_z + p) < (white_z - k))) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[i + black_x + q][black_y][j + black_z + p];
            }
          }
          --m_exchange_code_value_black
              [code_value];  //减去交换前由黑色交换点确定的模式
        }
      }
      //////////////////////////////////////////////////YZ平面交换点决定的模式
      if ((white_y + q >= k) && (white_y + q < m_vector_size - k) &&
          (white_z + p >= k) && (white_z + p < m_vector_size - k)) {
        code_value = 0;
        for (i = -k; i != k + 1; i++) {
          for (j = -k; j != k + 1; j++) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[white_x][i + white_y + q][j + white_z + p];
          }
        }
        --m_exchange_code_value_white
            [code_value];  //减去交换前由白色交换点确定的模式
      }
      if ((black_y + q >= k) && (q + black_y < m_vector_size - k) &&
          (black_z + p >= k) && (black_z + p < m_vector_size - k)) {
        if (white_x != black_x) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[black_x][i + black_y + q][j + black_z + p];
            }
          }
          --m_exchange_code_value_black
              [code_value];  //减去交换前由黑色交换点确定的模式
        } else if (((black_y + q) > (white_y + k)) ||
                   ((black_y + q) < (white_y - k)) ||
                   ((black_z + p) > (white_z + k)) ||
                   ((black_z + p) < (white_z - k))) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[black_x][i + black_y + q][j + black_z + p];
            }
          }
          --m_exchange_code_value_black
              [code_value];  //减去交换前由黑色交换点确定的模式
        }
      }
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      m_reconstruct_vec[white_x][white_y][white_z] = 0;  //交换后
      m_reconstruct_vec[black_x][black_y][black_z] = 1;
      ////////////////////////////////////////////////////XY平面交换点决定的模式
      if ((white_x + q >= k) && (white_x + q < m_vector_size - k) &&
          (white_y + p >= k) && (white_y + p < m_vector_size - k)) {
        code_value = 0;
        for (i = -k; i != k + 1; ++i) {
          for (j = -k; j != k + 1; ++j) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[i + white_x + q][j + white_y + p][white_z];
          }
        }
        pair_code_value = m_exchange_code_value_white.insert(
            make_pair(code_value, 1));  //加上交换前由白色交换点确定的模式
        if (!pair_code_value.second) {
          ++pair_code_value.first->second;
        }
      }

      if ((black_x + q >= k) && (q + black_x < m_vector_size - k) &&
          (black_y + p >= k) && (black_y + p < m_vector_size - k)) {
        if (white_z != black_z) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[i + black_x + q][j + black_y + p][black_z];
            }
          }
          pair_code_value =
              m_exchange_code_value_black.insert(make_pair(code_value, 1));
          if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
          {
            ++pair_code_value.first->second;
          }
        } else if (((black_x + q) > (white_x + k)) ||
                   ((black_x + q) < (white_x - k)) ||
                   ((black_y + p) > (white_y + k)) ||
                   ((black_y + p) < (white_y - k))) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[i + black_x + q][j + black_y + p][black_z];
            }
          }
          pair_code_value =
              m_exchange_code_value_black.insert(make_pair(code_value, 1));
          if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
          {
            ++pair_code_value.first->second;
          }
        }
      }

      ////////////////////////////////////////////////////YZ平面交换点决定的模式
      if ((white_z + q >= k) && (white_z + q < m_vector_size - k) &&
          (white_y + p >= k) && (white_y + p < m_vector_size - k)) {
        code_value = 0;
        for (i = -k; i != k + 1; ++i) {
          for (j = -k; j != k + 1; ++j) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[white_x][j + white_y + p][i + white_z + q];
          }
        }
        pair_code_value = m_exchange_code_value_white.insert(
            make_pair(code_value, 1));  //加上交换前由白色交换点确定的模式
        if (!pair_code_value.second) {
          ++pair_code_value.first->second;
        }
      }

      if ((black_z + q >= k) && (q + black_z < m_vector_size - k) &&
          (black_y + p >= k) && (black_y + p < m_vector_size - k)) {
        if (white_x != black_x) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[black_x][j + black_y + p][i + black_z + q];
            }
          }
          pair_code_value =
              m_exchange_code_value_black.insert(make_pair(code_value, 1));
          if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
          {
            ++pair_code_value.first->second;
          }
        } else if (((black_z + q) > (white_z + k)) ||
                   ((black_z + q) < (white_z - k)) ||
                   ((black_y + p) > (white_y + k)) ||
                   ((black_y + p) < (white_y - k))) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[black_x][j + black_y + p][i + black_z + q];
            }
          }
          pair_code_value =
              m_exchange_code_value_black.insert(make_pair(code_value, 1));
          if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
          {
            ++pair_code_value.first->second;
          }
        }
      }

      ////////////////////////////////////////////////////XZ平面交换点决定的模式
      if ((white_x + q >= k) && (white_x + q < m_vector_size - k) &&
          (white_z + p >= k) && (white_z + p < m_vector_size - k)) {
        code_value = 0;
        for (i = -k; i != k + 1; ++i) {
          for (j = -k; j != k + 1; ++j) {
            code_value =
                code_value * 2 +
                m_reconstruct_vec[i + white_x + q][white_y][j + white_z + p];
          }
        }
        pair_code_value = m_exchange_code_value_white.insert(
            make_pair(code_value, 1));  //加上交换前由白色交换点确定的模式
        if (!pair_code_value.second) {
          ++pair_code_value.first->second;
        }
      }

      if ((black_x + q >= k) && (q + black_x < m_vector_size - k) &&
          (black_z + p >= k) && (black_z + p < m_vector_size - k)) {
        if (white_y != black_y) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[i + black_x + q][black_y][j + black_z + p];
            }
          }
          pair_code_value =
              m_exchange_code_value_black.insert(make_pair(code_value, 1));
          if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
          {
            ++pair_code_value.first->second;
          }
        } else if (((black_x + q) > (white_x + k)) ||
                   ((black_x + q) < (white_x - k)) ||
                   ((black_z + p) > (white_z + k)) ||
                   ((black_z + p) < (white_z - k))) {
          code_value = 0;
          for (i = -k; i != k + 1; i++) {
            for (j = -k; j != k + 1; j++) {
              code_value =
                  code_value * 2 +
                  m_reconstruct_vec[i + black_x + q][black_y][j + black_z + p];
            }
          }
          pair_code_value =
              m_exchange_code_value_black.insert(make_pair(code_value, 1));
          if (!pair_code_value.second)  //加上交换前由黑色交换点确定的模式
          {
            ++pair_code_value.first->second;
          }
        }
      }
    }
  }
  int Engerry_before_exchange_white = 0;
  int Engerry_after_exchange_white = 0;
  int Engerry_before_exchange_black = 0;
  int Engerry_after_exchange_black = 0;
  map<int, int>::iterator iter_Success = m_exchange_code_value_white.begin();
  for (iter_Success; iter_Success != m_exchange_code_value_white.end();
       ++iter_Success) {
    Engerry_before_exchange_white =
        Engerry_before_exchange_white +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_vector_size *
                m_train_code_value[iter_Success
                                       ->first]);  //计算没有变化的模式个数
    m_reconstruct_code_value[iter_Success->first] =
        m_reconstruct_code_value[iter_Success->first] +
        m_exchange_code_value_white[iter_Success->first];  //保存原来的模式
    Engerry_after_exchange_white =
        Engerry_after_exchange_white +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_vector_size *
                m_train_code_value[iter_Success->first]);  //计算模式差值
  }
  Engerry_change_white =
      Engerry_after_exchange_white - Engerry_before_exchange_white;

  iter_Success = m_exchange_code_value_black.begin();
  for (iter_Success; iter_Success != m_exchange_code_value_black.end();
       ++iter_Success) {
    Engerry_before_exchange_black =
        Engerry_before_exchange_black +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_vector_size *
                m_train_code_value[iter_Success
                                       ->first]);  //计算没有变化的模式个数
    m_reconstruct_code_value[iter_Success->first] =
        m_reconstruct_code_value[iter_Success->first] +
        m_exchange_code_value_black[iter_Success->first];  //保存原来的模式
    Engerry_after_exchange_black =
        Engerry_after_exchange_black +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_vector_size *
                m_train_code_value[iter_Success->first]);  //计算模式差值
  }
  Engerry_change_black =
      Engerry_after_exchange_black - Engerry_before_exchange_black;

  m_Enerry_Reverse = m_Enerry_origial;  //将交换前的能量差保存，以便替换回来
  Engerry_down = m_Enerry_origial + Engerry_change_white +
                 Engerry_change_black;  //传递交换后的能量差
  m_Enerry_origial = Engerry_down;      //将当期能量差作为新状态
  m_reconstruct_vec[white_x][white_y][white_z] = 0;  //交换后
  m_reconstruct_vec[black_x][black_y][black_z] = 1;
}

void CAnnealing::Select_final_exchange_point_uncondition_fast(
    int& white_point_number, int& black_point_number) {
  int i = 0, j = 0, k = 0, p = 0, q = 0, g = 0, n = 0;
  int sum_white = 0, sum_black = 0;
  vector<vector<vector<int>>> temporay_vector;
  temporay_vector.resize(m_vector_size);
  for (i = 0; i != m_vector_size; i++) {
    temporay_vector[i].resize(m_vector_size);
    for (j = 0; j != m_vector_size; j++) {
      temporay_vector[i][j].resize(m_vector_size);
      for (k = 0; k != m_vector_size; k++) {
        temporay_vector[i][j][k] = 2;
      }
    }
  }

  ///判断将上层网格点对应当前网格点像素值相反的网格点作为交换点，并且排除上层作为全零和全1的情况
  for (i = 0; i != m_vector_size; i++) {
    for (j = 0; j != m_vector_size; j++) {
      for (k = 0; k != m_vector_size; k++) {
        if (i % 2 == 0 && j % 2 == 0 && k % 2 == 0) {
          if (m_reconstruct_vec[i][j][k] == 1) {
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                for (g = -1; g != 2; g++) {
                  if (m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                       [(j + p + m_vector_size) % m_vector_size]
                                       [(k + g + m_vector_size) %
                                        m_vector_size] == 0) {
                    temporay_vector[(i + q + m_vector_size) % m_vector_size]
                                   [(j + p + m_vector_size) % m_vector_size]
                                   [(k + g + m_vector_size) % m_vector_size] =
                                       0;
                  }
                }
              }
            }
          } else {
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                for (g = -1; g != 2; g++) {
                  if (m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                       [(j + p + m_vector_size) % m_vector_size]
                                       [(k + g + m_vector_size) %
                                        m_vector_size] == 1) {
                    temporay_vector[(i + q + m_vector_size) % m_vector_size]
                                   [(j + p + m_vector_size) % m_vector_size]
                                   [(k + g + m_vector_size) % m_vector_size] =
                                       1;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  //////////////////////////////////////////////////////////选取交换点
  white_point_number = 0;
  black_point_number = 0;
  for (i = 0; i != m_vector_size; i++) {
    for (j = 0; j != m_vector_size; j++) {
      for (k = 0; k != m_vector_size; k++) {
        if (temporay_vector[i][j][k] == 1) {
          m_white_posx.push_back(i);
          m_white_posy.push_back(j);
          m_white_posz.push_back(k);
          ++white_point_number;
        } else if (temporay_vector[i][j][k] == 0) {
          m_black_posx.push_back(i);
          m_black_posy.push_back(j);
          m_black_posz.push_back(k);
          ++black_point_number;
        }
      }
    }
  }
  m_class_white_point_number = white_point_number;
  m_class_black_point_number = black_point_number;
  temporay_vector.clear();
}

void CAnnealing::Set_inital_to_reconstruct_uncondition_3() {
  int i = 0, j = 0, k = 0, p = 0, q = 0;
  int sum = 0;
  int up_grid_white_number = 0;
  int current_grid_white_number = 0;
  int inital_white_number = 0;
  int add_white_number = 0;
  int up_white_number = 0;
  int distance_point = m_initial_width / m_vector_size;
  /////////////////////////////////////////////////给判决容器赋初值，将上层图像传递给重建容器；
  m_reconstruct_vec.resize(m_vector_size);
  for (i = 0; i != m_vector_size; ++i) {
    m_reconstruct_vec[i].resize(m_vector_size);
    for (j = 0; j != m_vector_size; ++j) {
      m_reconstruct_vec[i][j].resize(m_vector_size);
      for (k = 0; k != m_vector_size; k++) {
        if (i % 2 == 0 && j % 2 == 0 && k % 2 == 0) {
          m_reconstruct_vec[i][j][k] =
              m_initial_rand_data[i / 2][j / 2]
                                 [k / 2];  /////将上层重建的结果传递到对应网格点
          if (m_initial_rand_data[i / 2][j / 2][k / 2] == 1) {
            up_white_number++;
          }
        } else {
          m_reconstruct_vec[i][j][k] = 2;  /////其余点赋初值2
        }
      }
    }
  }

  for (i = 0; i != m_vector_size; ++i) {
    for (j = 0; j != m_vector_size; ++j) {
      if (m_initial_imgvec[i * distance_point][j * distance_point] ==
          1)  /////统计对应网格白色点数
      {
        inital_white_number++;
      }
    }
  }
  add_white_number = m_vector_size * inital_white_number - up_white_number;
  int point_x = 0;
  int point_y = 0;
  int point_z = 0;
  /////////////////////////////////////////////////////////给重建容器其余位置按照孔隙度随机赋值；
  while (add_white_number != 0) {
    point_x = rand32() % m_vector_size;
    point_y = rand32() % m_vector_size;
    point_z = rand32() % m_vector_size;
    if ((point_y % 2 == 1 || point_x % 2 == 1 || point_z % 2 == 1) &&
        m_reconstruct_vec[point_x][point_y][point_z] == 2) {
      m_reconstruct_vec[point_x][point_y][point_z] = 1;
      add_white_number--;
    }
  }

  for (i = 0; i != m_vector_size; i++) {
    for (j = 0; j != m_vector_size; j++) {
      for (k = 0; k != m_vector_size; k++) {
        if ((i % 2 == 1 || j % 2 == 1 || k % 2 == 1) &&
            m_reconstruct_vec[i][j][k] == 2) {
          m_reconstruct_vec[i][j][k] = 0;
        }
      }
    }
  }
}

//
// void CAnnealing::Select_final_exchange_point_uncondition_test(int&
// white_point_number,int& black_point_number)
//{
//	int i=0,j=0,k=0,p=0,q=0,g=0,n=0;
//	int sum_xy=0;
//	int sum_xz=0;
//	int sum_yz=0;
//	vector<vector<vector<int>>> temporay_vector;
//	temporay_vector.resize(m_vector_size);
//	for (i=0;i!=m_vector_size;i++)
//	{
//		temporay_vector[i].resize(m_vector_size);
//		for (j=0;j!=m_vector_size;j++)
//		{
//			temporay_vector[i][j].resize(m_vector_size);
//			for (k=0;k!=m_vector_size;k++)
//			{
//				temporay_vector[i][j][k]=2;
//			}
//		}
//	}
//
//	///相邻网格反相选点法，且排除上层作为全零和全1的情况
//	for (i=0;i!=m_vector_size;i++)
//	{
//		for (j=0;j!=m_vector_size;j++)
//		{
//			for (k=0;k!=m_vector_size;k++)
//			{
//				if (i%2==0 && j%2==0 && k%2==0)
//				{
//					if (m_reconstruct_vec[i][j][k]==1)
//					{
//						for (q=-1;q!=2;q++)
//						{
//							for (p=-1;p!=2;p++)
//							{
//								for
//(g=-1;g!=2;g++)
//								{
//									if
//(m_reconstruct_vec[(i+q+m_vector_size)%m_vector_size][(j+p+m_vector_size)%m_vector_size][(k+g+m_vector_size)%m_vector_size]==0
//&&
// m_preproc_vector[(i+q+m_vector_size)%m_vector_size][(j+p+m_vector_size)%m_vector_size][(k+g+m_vector_size)%m_vector_size]==2
//)
//									{
//										temporay_vector[(i+q+m_vector_size)%m_vector_size][(j+p+m_vector_size)%m_vector_size][(k+g+m_vector_size)%m_vector_size]=0;
//									}
//								}
//							}
//						}
//					}
//					else
//					{
//						for (q=-1;q!=2;q++)
//						{
//							for (p=-1;p!=2;p++)
//							{
//								for
//(g=-1;g!=2;g++)
//								{
//									if
//(m_reconstruct_vec[(i+q+m_vector_size)%m_vector_size][(j+p+m_vector_size)%m_vector_size][(k+g+m_vector_size)%m_vector_size]==1
//&&
// m_preproc_vector[(i+q+m_vector_size)%m_vector_size][(j+p+m_vector_size)%m_vector_size][(k+g+m_vector_size)%m_vector_size]==2)
//									{
//										temporay_vector[(i+q+m_vector_size)%m_vector_size][(j+p+m_vector_size)%m_vector_size][(k+g+m_vector_size)%m_vector_size]=1;
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//	///选取不连续的黑色像素点
//	for (i=0;i!=m_vector_size;i++)
//	{
//		for (j=0;j!=m_vector_size;j++)
//		{
//			for (k=0;k!=m_vector_size;k++)
//			{
//				sum_xy=0;
//				sum_xz=0;
//				sum_yz=0;
//				for (p=-1;p!=2;p++)
//				{
//					for (q=-1;q!=2;q++)
//					{
//						sum_xy=sum_xy+m_reconstruct_vec[(i+p+m_vector_size)%m_vector_size][(j+q+m_vector_size)%m_vector_size][k];
//					}
//				}
//				for (p=-1;p!=2;p++)
//				{
//					for (q=-1;q!=2;q++)
//					{
//						sum_xz=sum_xz+m_reconstruct_vec[(i+p+m_vector_size)%m_vector_size][j][(k+q+m_vector_size)%m_vector_size];
//					}
//				}
//				for (p=-1;p!=2;p++)
//				{
//					for (q=-1;q!=2;q++)
//					{
//						sum_yz=sum_yz+m_reconstruct_vec[i][(j+p+m_vector_size)%m_vector_size][(k+q+m_vector_size)%m_vector_size];
//					}
//				}
//				if ((sum_xy==8 || sum_xz==8 || sum_yz==8 ) &&
// m_reconstruct_vec[i][j][k]==0 && m_preproc_vector[i][j][k]==2)
//				{
//					temporay_vector[i][j][k]=0;
//				}
//			}
//		}
//	}
//	//////////////////////////////////////////////////////////选取交换点
//	white_point_number=0;
//	black_point_number=0;
//	for (i=0;i!=m_vector_size;i++)
//	{
//		for (j=0;j!=m_vector_size;j++)
//		{
//			for (k=0;k!=m_vector_size;k++)
//			{
//				if (temporay_vector[i][j][k]==1 )
//				{
//					white_point_x.push_back(i);
//					white_point_y.push_back(j);
//					white_point_z.push_back(k);
//					++white_point_number;
//				}
//				else if (temporay_vector[i][j][k]==0 )
//				{
//					black_point_x.push_back(i);
//					black_point_y.push_back(j);
//					black_point_z.push_back(k);
//					++black_point_number;
//				}
//			}
//		}
//	}
//
//	class_white_point_number=white_point_number;
//	class_black_point_number=black_point_number;
//	temporay_vector.clear();
//}

void CAnnealing::Select_final_exchange_point_uncondition_test(
    int& white_point_number, int& black_point_number) {
  int i = 0, j = 0, k = 0, p = 0, q = 0, g = 0, n = 0;
  vector<vector<vector<int>>> temporay_vector;
  temporay_vector.resize(m_vector_size);
  for (i = 0; i != m_vector_size; i++) {
    temporay_vector[i].resize(m_vector_size);
    for (j = 0; j != m_vector_size; j++) {
      temporay_vector[i][j].resize(m_vector_size);
      for (k = 0; k != m_vector_size; k++) {
        temporay_vector[i][j][k] = 2;
      }
    }
  }

  ///判断将上层网格点对应当前网格点像素值相反的网格点作为交换点，并且排除上层作为全零和全1的情况
  int sum_xy = 0;
  int sum_xz = 0;
  int sum_yz = 0;
  int sum_3D = 0;
  for (i = 0; i != m_vector_size; i++) {
    for (j = 0; j != m_vector_size; j++) {
      for (k = 0; k != m_vector_size; k++) {
        /////////////////////////////////////////////////////////////////////
        if (i % 2 == 1 && j % 2 == 1 && k % 2 == 1 &&
            m_preproc_vector[i][j][k] == 2) {
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy =
                  sum_xy +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                   [(j + p + m_vector_size) % m_vector_size][k];
              sum_xz =
                  sum_xz +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size][j]
                                   [(k + p + m_vector_size) % m_vector_size];
              sum_yz =
                  sum_yz +
                  m_reconstruct_vec[i][(j + p + m_vector_size) % m_vector_size]
                                   [(k + p + m_vector_size) % m_vector_size];
            }
          }
          if (m_reconstruct_vec[i][j][k] == 1 && sum_xy != 9 && sum_xz != 9 &&
              sum_yz != 9) {
            temporay_vector[i][j][k] = 1;
          } else if (m_reconstruct_vec[i][j][k] == 0 && sum_xy != 0 &&
                     sum_xz != 0 && sum_yz != 0) {
            temporay_vector[i][j][k] = 0;
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 1 && j % 2 == 1 && k % 2 == 0 &&
                 m_preproc_vector[i][j][k] == 2) {
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy =
                  sum_xy +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                   [(j + p + m_vector_size) % m_vector_size][k];
              sum_xz =
                  sum_xz +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size][j]
                                   [(k + p + m_vector_size) % m_vector_size];
              sum_yz =
                  sum_yz +
                  m_reconstruct_vec[i][(j + q + m_vector_size) % m_vector_size]
                                   [(k + p + m_vector_size) % m_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D =
                    sum_3D +
                    m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                     [(j + p + m_vector_size) % m_vector_size]
                                     [(k + g + m_vector_size) % m_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (sum_xy != 9 && (sum_xz != 9 || sum_yz != 9) &&
                m_reconstruct_vec[i][j][k] == 1) {
              temporay_vector[i][j][k] = 1;
            } else if (sum_xy != 0 && (sum_xz != 0 || sum_yz != 0) &&
                       m_reconstruct_vec[i][j][k] == 0) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 1 && j % 2 == 0 && k % 2 == 1 &&
                 m_preproc_vector[i][j][k] == 2) {
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy =
                  sum_xy +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                   [(j + p + m_vector_size) % m_vector_size][k];
              sum_xz =
                  sum_xz +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size][j]
                                   [(k + p + m_vector_size) % m_vector_size];
              sum_yz =
                  sum_yz +
                  m_reconstruct_vec[i][(j + q + m_vector_size) % m_vector_size]
                                   [(k + p + m_vector_size) % m_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D =
                    sum_3D +
                    m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                     [(j + p + m_vector_size) % m_vector_size]
                                     [(k + g + m_vector_size) % m_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (sum_xz != 9 && (sum_xy != 9 || sum_yz != 9) &&
                m_reconstruct_vec[i][j][k] == 1) {
              temporay_vector[i][j][k] = 1;
            } else if (sum_xz != 0 && (sum_xy != 0 || sum_yz != 0) &&
                       m_reconstruct_vec[i][j][k] == 0) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 0 && j % 2 == 1 && k % 2 == 1 &&
                 m_preproc_vector[i][j][k] == 2) {
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy =
                  sum_xy +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                   [(j + p + m_vector_size) % m_vector_size][k];
              sum_xz =
                  sum_xz +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size][j]
                                   [(k + p + m_vector_size) % m_vector_size];
              sum_yz =
                  sum_yz +
                  m_reconstruct_vec[i][(j + q + m_vector_size) % m_vector_size]
                                   [(k + p + m_vector_size) % m_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D =
                    sum_3D +
                    m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                     [(j + p + m_vector_size) % m_vector_size]
                                     [(k + g + m_vector_size) % m_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (sum_yz != 9 && (sum_xy != 9 || sum_xz != 9) &&
                m_reconstruct_vec[i][j][k] == 1) {
              temporay_vector[i][j][k] = 1;
            } else if (sum_yz != 0 && (sum_xy != 0 || sum_xz != 0) &&
                       m_reconstruct_vec[i][j][k] == 0) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 0 && j % 2 == 0 && k % 2 == 1 &&
                 m_preproc_vector[i][j][k] == 2) {
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy =
                  sum_xy +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                   [(j + p + m_vector_size) % m_vector_size][k];
              sum_xz =
                  sum_xz +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size][j]
                                   [(k + p + m_vector_size) % m_vector_size];
              sum_yz =
                  sum_yz +
                  m_reconstruct_vec[i][(j + p + m_vector_size) % m_vector_size]
                                   [(k + p + m_vector_size) % m_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D =
                    sum_3D +
                    m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                     [(j + p + m_vector_size) % m_vector_size]
                                     [(k + g + m_vector_size) % m_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (m_reconstruct_vec[i][j][k] == 1 &&
                (sum_xy != 9 || (sum_xz != 9 && sum_yz != 9))) {
              temporay_vector[i][j][k] = 1;
            } else if (m_reconstruct_vec[i][j][k] == 0 &&
                       (sum_xy != 0 || (sum_xz != 0 && sum_yz != 0))) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 0 && j % 2 == 1 && k % 2 == 0 &&
                 m_preproc_vector[i][j][k] == 2) {
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy =
                  sum_xy +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                   [(j + p + m_vector_size) % m_vector_size][k];
              sum_xz =
                  sum_xz +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size][j]
                                   [(k + p + m_vector_size) % m_vector_size];
              sum_yz =
                  sum_yz +
                  m_reconstruct_vec[i][(j + p + m_vector_size) % m_vector_size]
                                   [(k + p + m_vector_size) % m_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D =
                    sum_3D +
                    m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                     [(j + p + m_vector_size) % m_vector_size]
                                     [(k + g + m_vector_size) % m_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (m_reconstruct_vec[i][j][k] == 1 &&
                (sum_xz != 9 || (sum_xy != 9 && sum_yz != 9))) {
              temporay_vector[i][j][k] = 1;
            } else if (m_reconstruct_vec[i][j][k] == 0 &&
                       (sum_xz != 0 || (sum_xy != 0 && sum_yz != 0))) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }

        /////////////////////////////////////////////////////////////////////
        else if (i % 2 == 1 && j % 2 == 0 && k % 2 == 0 &&
                 m_preproc_vector[i][j][k] == 2) {
          sum_3D = 0;
          sum_xy = 0;
          sum_xz = 0;
          sum_yz = 0;
          for (q = -1; q != 2; q++) {
            for (p = -1; p != 2; p++) {
              sum_xy =
                  sum_xy +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                   [(j + p + m_vector_size) % m_vector_size][k];
              sum_xz =
                  sum_xz +
                  m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size][j]
                                   [(k + p + m_vector_size) % m_vector_size];
              sum_yz =
                  sum_yz +
                  m_reconstruct_vec[i][(j + p + m_vector_size) % m_vector_size]
                                   [(k + p + m_vector_size) % m_vector_size];
              for (g = -1; g != 2; g++) {
                sum_3D =
                    sum_3D +
                    m_reconstruct_vec[(i + q + m_vector_size) % m_vector_size]
                                     [(j + p + m_vector_size) % m_vector_size]
                                     [(k + g + m_vector_size) % m_vector_size];
              }
            }
          }
          if (sum_3D == 27) {
            m_preproc_vector[i][j][k] = 1;
          } else if (sum_3D == 0) {
            m_preproc_vector[i][j][k] = 0;
          } else {
            if (m_reconstruct_vec[i][j][k] == 1 &&
                (sum_yz != 9 || (sum_xy != 9 && sum_xz != 9))) {
              temporay_vector[i][j][k] = 1;
            } else if (m_reconstruct_vec[i][j][k] == 0 &&
                       (sum_yz != 0 || (sum_xy != 0 && sum_xz != 0))) {
              temporay_vector[i][j][k] = 0;
            }
          }
        }
      }
    }
  }

  ///选取不连续的黑色像素点
  for (i = 0; i != m_vector_size; i++) {
    for (j = 0; j != m_vector_size; j++) {
      for (k = 0; k != m_vector_size; k++) {
        sum_xy = 0;
        sum_xz = 0;
        sum_yz = 0;
        for (p = -1; p != 2; p++) {
          for (q = -1; q != 2; q++) {
            sum_xy =
                sum_xy +
                m_reconstruct_vec[(i + p + m_vector_size) % m_vector_size]
                                 [(j + q + m_vector_size) % m_vector_size][k];
          }
        }
        for (p = -1; p != 2; p++) {
          for (q = -1; q != 2; q++) {
            sum_xz =
                sum_xz +
                m_reconstruct_vec[(i + p + m_vector_size) % m_vector_size][j]
                                 [(k + q + m_vector_size) % m_vector_size];
          }
        }
        for (p = -1; p != 2; p++) {
          for (q = -1; q != 2; q++) {
            sum_yz =
                sum_yz +
                m_reconstruct_vec[i][(j + p + m_vector_size) % m_vector_size]
                                 [(k + q + m_vector_size) % m_vector_size];
          }
        }
        if ((sum_xy == 8 || sum_xz == 8 || sum_yz == 8) &&
            m_reconstruct_vec[i][j][k] == 0 && m_preproc_vector[i][j][k] == 2) {
          temporay_vector[i][j][k] = 0;
        }
      }
    }
  }
  // temporay_vector存放选择交换点
  // Sign_exchange_vector存放交换成功与否情况
  // m_preproc_vector存放已经确定的点，不再参与交换

  //////////////////////////////////////////////////////////选取交换点
  white_point_number = 0;
  black_point_number = 0;
  bool judgement = false;

  for (i = 0; i != m_vector_size; i++) {
    for (j = 0; j != m_vector_size; j++) {
      for (k = 0; k != m_vector_size; k++) {
        if (temporay_vector[i][j][k] == 1) {
          m_white_posx.push_back(i);
          m_white_posy.push_back(j);
          m_white_posz.push_back(k);
          ++white_point_number;
        } else if (temporay_vector[i][j][k] == 0) {
          m_black_posx.push_back(i);
          m_black_posy.push_back(j);
          m_black_posz.push_back(k);
          ++black_point_number;
        }
      }
    }
  }

  m_class_white_point_number = white_point_number;
  m_class_black_point_number = black_point_number;
  temporay_vector.clear();
}

