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
  m_Rec_template_size = template_size;  //�����������ݲ���ģ���С������
  m_sizenum = (int)pow(2.0, pow(m_Rec_template_size, 2.0));
  m_Enerry_origial = 0;
  ;
  m_Enerry_Reverse = 0;
  m_final_imgsize = final_imgsz;

  //����ͼƬ��m_initial_imgvec
  LoadInitialImg(imgpath);
  m_multiple = m_final_imgsize / m_initial_width;

  //ȡ��������־
  m_ShutDown = false;

}

void CAnnealing::SetSavePath(const QString& dstimgpath) {
  m_dstimg_path = dstimgpath;
}

bool CAnnealing::Reconstruct() {
  m_Rec_template_size = 3;  //ѡ�����ܶȺ���ģ��ĳߴ��С,Ĭ��Ϊ3
  m_Rec_min_grid_size = 16;  //��С����ʱͼ��ĳߴ��С,Ĭ��16
  //����Ҫ����չ����
  int grid_times =
      (log((float)m_initial_width) - log((float)m_Rec_min_grid_size)) /
      log(2.0);
  //�ȶ���һ����ά���������ڴ�ų�ʼ��С����
  //(�����õ���ά���ٸ��ݿ�϶�ȣ�inital_white_point*(m_multiple*m_multiple)*new_inital_size������ά����ֵ������m_initial_rand_data��
  //����������������Set_inital_to_reconstruct_uncondition_1���ʹ��
  // Set_inital_data();
  Set_inital_data_Recons();

  int judgements_times = 0;
  int energy_up = 0;             //����ǰ��������С
  int energy_down = 0;           //�������������С
  int energy_differ = 0;         //�������ε�������
  int energy_jump = 0;           //�������������
  int energy_judge = 0;          //��ѭ����������
  int percent = 0.0005;          //������ٷֱ�,Ĭ��0.0005�����޸�
  int adjust_time = 0;           //�жϼ�������Ƿ����
  int refuse_times_out = 0;      //�ܾ�����
  int max_refuse_times_out = 1;  //���ܾ�����
  int loop_in = 0;               //�����ڲ�ѭ��
  int loop_out = 0;              //�����ⲿѭ��
  int loop_in_times = 0;         //�ڲ�ѭ������
  int loop_out_times = 200;      //�ⲿѭ������
  int main_white_point_number = 0;  //ѡ���ɫ����������
  int main_black_point_number = 0;  //ѡ���ɫ����������

  int energy_process = 0;
  int engerry_change_white = 0;
  int engerry_change_black = 0;
  int judgement_times = 0;

  int prgblock = 100 / (grid_times + 1);
  for (int i = 0; i != grid_times + 1; ++i) {
    if (m_ShutDown) return false;
    //���ͽ���
    int currprg = i * prgblock;
    emit CurrProgress(currprg);
    
    judgement_times = 0;
    Transfer_grid_size(i);  //��ȡ��ǰ�����ͼ��ߴ��m_new_vector_size
    TI_Multi_point_density_function_p();  //�����Ӧ�����СΪiʱͼ��Ķ���ܶȺ���
    Putout_inital_image(m_dstimg_path);  //���ÿһ��ԭʼͼ���Ӧ����ͼ��
    if (i == 0) {
      //��������ݷ��õ��ؽ�����m_reconstruct_vec[i][j][k]��,����ʼ�ߴ�
      //�󲿷ֹ����Ѿ���Set_inital_data��������
      Set_inital_to_reconstruct_uncondition_1();
    } else {
      //�Ƚ��ϼ��������ȫ1��ȫ0λ���ڵ�ǰ����������Ϊȫ1��ȫ0���ӵ�һ����ά��ʼ���Ѿ��˹������ˣ������������������
      //����TI�а�ɫ�����������
      Set_inital_to_reconstruct_uncondition_2_Recons();
    }
    Recstruct_Multi_point_density_function_p();  //�����Ӧ�����СΪiʱ�ؽ�ͼ��Ķ���ܶȺ���
    Calculate_E_differ(energy_up);  //�������ܶȺ���������ֵ
    // cout << "origial_Energy_up:" << Energy_up << endl;   //�ղ��õ���energy
    // energy_limit = percent*Energy_up;    // ���energy_limitû���õ��� from
    // wrh

    energy_judge = energy_up;
    adjust_time = energy_up;
    energy_jump = 0;

    float inner_prgblock = prgblock * 1.0 / loop_out_times;  //wrh
    for (loop_out = 0; loop_out < loop_out_times; loop_out++) {
      if (m_ShutDown) return false;
      emit CurrProgress(currprg + loop_out * inner_prgblock);  //wrh
      if (i <= (grid_times + 1) / 2)  //���ò�ʹ�����ڷ���ѡ�㷨�����缶��
      {
        //���е㶼�������Ѵ������ĵ�ѡ��ú��������white_point_x��������
        Select_inital_exchange_point_uncondition_Recons(main_white_point_number,
                                                 main_black_point_number);
      } else {
        // temperory�зŽ����ĵ㣬Ҳ�Ƿ���white_point_x��������
        Select_final_exchange_point_uncondition_Recons(main_white_point_number,
                                                main_black_point_number);
        energy_process = energy_up;
      }
      Random_position(loop_out);
      //�����ǲ�ͬ���֮�����λ����������Ӧ����������������Ϊ������������
      if (main_black_point_number > main_white_point_number) {
        loop_in_times = main_white_point_number;
      } else {
        loop_in_times = main_black_point_number;
      }
      for (loop_in = 0; loop_in != loop_in_times; loop_in++)  //�������ڽ���
      {
        if (m_ShutDown) return false;
        if (main_white_point_number != 0 && main_black_point_number != 0) {
          //ȷ����white_point_x(black)�е�ƫ��λ��white_rand_site
          //�ӱ�ѡ�㼯�����������������
          Exchange_two_point();
          Get_exchange_site();  //�õ�������������꣬Ϊ����ʹ��
                                //���ټ������ܶȺ���
          Recstruct_Multi_point_density_function_fast_p(
              energy_down, engerry_change_white, engerry_change_black);

          energy_differ = energy_down - energy_up;
          if (energy_differ <= 0) {
            energy_up = energy_down;  //���µĽ�С��������ΪEnergy_up
            Delete_exchange_point();  //��������ĵ�ɾ��������ǳ�����
            main_black_point_number--;
            main_white_point_number--;
          } else if (judgement_times >= 1) {
            energy_up = energy_down;
            Delete_exchange_point();  //��������ĵ�ɾ��������ǳ�����
            main_black_point_number--;
            main_white_point_number--;
            judgement_times = 0;
          } else {  //˵����ǰλ�õİ׵㲻�ܸı���λ��ռ��ͷ�����ܽ��������ڵ���Ҫ�ı䣬����Ҫ����һ���׵���н��������Բ�delete��
            if (engerry_change_white > 0 && engerry_change_black < 0) {
              Recover_Multi_point_density_function();  //�ָ���ǰһ״̬�Ķ���ܶȺ���
              Reverse_exchange_point();  //�������������Ľ����㸴ԭ��
              Delete_unexchange_white_point();  //��������ĵ�ɾ����
              main_white_point_number--;
            } else if (engerry_change_white < 0 && engerry_change_black > 0) {
              Recover_Multi_point_density_function();  //�ָ���ǰһ״̬�Ķ���ܶȺ���
              Reverse_exchange_point();  //�������������Ľ����㸴ԭ��
              Delete_unexchange_black_point();  //��������ĵ�ɾ����
              main_black_point_number--;
            } else {
              Recover_Multi_point_density_function();  //�ָ���ǰһ״̬�Ķ���ܶȺ���
              Reverse_exchange_point();  //�������������Ľ����㸴ԭ��
              Delete_unexchange_two_phase_point();  //��������ĵ�ɾ����
              main_black_point_number--;
              main_white_point_number--;
            }
          }
        }
        if (main_white_point_number == 0 || main_black_point_number == 0) {
          break;
        }
      }
      Delete_site_vector();  //�����ֽ������������գ��Ա���һ��ѭ����
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
    Set_reconstruct_to_inital();  //��reconstructed�е��м��ؽ����ת�Ƶ�inital
    Putout_step_image(m_dstimg_path);  //���ԭʼͼ���Ӧ����ͼ��
  }

  return true;
}

bool CAnnealing::TwoFuseThree() { 
  
  m_Rec_template_size = 3;  //ѡ�����ܶȺ���ģ��ĳߴ��С,Ĭ��Ϊ3
  m_Rec_min_grid_size = 16;  //��С����ʱͼ��ĳߴ��С,Ĭ��16
  //����Ҫ����չ����
  int grid_times =
      (log((float)m_initial_width) - log((float)m_Rec_min_grid_size)) /
      log(2.0);
  //�ȶ���һ����ά���������ڴ�ų�ʼ��С����
  //(�����õ���ά���ٸ��ݿ�϶�ȣ�inital_white_point*(m_multiple*m_multiple)*new_inital_size������ά����ֵ������m_initial_rand_data��
  //����������������Set_inital_to_reconstruct_uncondition_1���ʹ��
  // Set_inital_data();
  Set_inital_data_2fuse3();

  int judgements_times = 0;
  int energy_up = 0;             //����ǰ��������С
  int energy_down = 0;           //�������������С
  int energy_differ = 0;         //�������ε�������
  int energy_jump = 0;           //�������������
  int energy_judge = 0;          //��ѭ����������
  int percent = 0.0005;          //������ٷֱ�,Ĭ��0.0005�����޸�
  int adjust_time = 0;           //�жϼ�������Ƿ����
  int refuse_times_out = 0;      //�ܾ�����
  int max_refuse_times_out = 1;  //���ܾ�����
  int loop_in = 0;               //�����ڲ�ѭ��
  int loop_out = 0;              //�����ⲿѭ��
  int loop_in_times = 0;         //�ڲ�ѭ������
  int loop_out_times = 200;      //�ⲿѭ������
  int main_white_point_number = 0;  //ѡ���ɫ����������
  int main_black_point_number = 0;  //ѡ���ɫ����������

  int energy_process = 0;
  int engerry_change_white = 0;
  int engerry_change_black = 0;
  int judgement_times = 0;

  int prgblock = 100 / (grid_times + 1);
  for (int i = 0; i != grid_times + 1; ++i) {
    if (m_ShutDown) return false;
    //���ͽ���
    int currprg = i * prgblock;
    emit CurrProgress(currprg);
    
    judgement_times = 0;
    Transfer_grid_size(i);  //��ȡ��ǰ�����ͼ��ߴ��m_new_vector_size
    TI_Multi_point_density_function_p();  //�����Ӧ�����СΪiʱͼ��Ķ���ܶȺ���
    Putout_inital_image(m_dstimg_path);  //���ÿһ��ԭʼͼ���Ӧ����ͼ��
    if (i == 0) {
      //��������ݷ��õ��ؽ�����m_reconstruct_vec[i][j][k]��,����ʼ�ߴ�
      //�󲿷ֹ����Ѿ���Set_inital_data��������
      Set_inital_to_reconstruct_uncondition_1();
    } else {
      //�Ƚ��ϼ��������ȫ1��ȫ0λ���ڵ�ǰ����������Ϊȫ1��ȫ0���ӵ�һ����ά��ʼ���Ѿ��˹������ˣ������������������
      //����TI�а�ɫ�����������
      Set_inital_to_reconstruct_uncondition_2_2fuse3();
    }
    Recstruct_Multi_point_density_function_p();  //�����Ӧ�����СΪiʱ�ؽ�ͼ��Ķ���ܶȺ���
    Calculate_E_differ(energy_up);  //�������ܶȺ���������ֵ
    // cout << "origial_Energy_up:" << Energy_up << endl;   //�ղ��õ���energy
    // energy_limit = percent*Energy_up;    // ���energy_limitû���õ��� from
    // wrh

    energy_judge = energy_up;
    adjust_time = energy_up;
    energy_jump = 0;

    float inner_prgblock = prgblock * 1.0 / loop_out_times;  //wrh
    for (loop_out = 0; loop_out < loop_out_times; loop_out++) {
      if (m_ShutDown) return false;
      emit CurrProgress(currprg + loop_out * inner_prgblock);  //wrh
      if (i <= (grid_times + 1) / 2)  //���ò�ʹ�����ڷ���ѡ�㷨�����缶��
      {
        //���е㶼�������Ѵ������ĵ�ѡ��ú��������white_point_x��������
        Select_inital_exchange_point_uncondition_2fuse3(main_white_point_number,
                                                 main_black_point_number);
      } else {
        // temperory�зŽ����ĵ㣬Ҳ�Ƿ���white_point_x��������
        Select_final_exchange_point_uncondition_2fuse3(main_white_point_number,
                                                main_black_point_number);
        energy_process = energy_up;
      }
      Random_position(loop_out);
      //�����ǲ�ͬ���֮�����λ����������Ӧ����������������Ϊ������������
      if (main_black_point_number > main_white_point_number) {
        loop_in_times = main_white_point_number;
      } else {
        loop_in_times = main_black_point_number;
      }
      for (loop_in = 0; loop_in != loop_in_times; loop_in++)  //�������ڽ���
      {
        if (m_ShutDown) return false;
        if (main_white_point_number != 0 && main_black_point_number != 0) {
          //ȷ����white_point_x(black)�е�ƫ��λ��white_rand_site
          //�ӱ�ѡ�㼯�����������������
          Exchange_two_point();
          Get_exchange_site();  //�õ�������������꣬Ϊ����ʹ��
                                //���ټ������ܶȺ���
          Recstruct_Multi_point_density_function_fast_p(
              energy_down, engerry_change_white, engerry_change_black);

          energy_differ = energy_down - energy_up;
          if (energy_differ <= 0) {
            energy_up = energy_down;  //���µĽ�С��������ΪEnergy_up
            Delete_exchange_point();  //��������ĵ�ɾ��������ǳ�����
            main_black_point_number--;
            main_white_point_number--;
          } else if (judgement_times >= 1) {
            energy_up = energy_down;
            Delete_exchange_point();  //��������ĵ�ɾ��������ǳ�����
            main_black_point_number--;
            main_white_point_number--;
            judgement_times = 0;
          } else {  //˵����ǰλ�õİ׵㲻�ܸı���λ��ռ��ͷ�����ܽ��������ڵ���Ҫ�ı䣬����Ҫ����һ���׵���н��������Բ�delete��
            if (engerry_change_white > 0 && engerry_change_black < 0) {
              Recover_Multi_point_density_function();  //�ָ���ǰһ״̬�Ķ���ܶȺ���
              Reverse_exchange_point();  //�������������Ľ����㸴ԭ��
              Delete_unexchange_white_point();  //��������ĵ�ɾ����
              main_white_point_number--;
            } else if (engerry_change_white < 0 && engerry_change_black > 0) {
              Recover_Multi_point_density_function();  //�ָ���ǰһ״̬�Ķ���ܶȺ���
              Reverse_exchange_point();  //�������������Ľ����㸴ԭ��
              Delete_unexchange_black_point();  //��������ĵ�ɾ����
              main_black_point_number--;
            } else {
              Recover_Multi_point_density_function();  //�ָ���ǰһ״̬�Ķ���ܶȺ���
              Reverse_exchange_point();  //�������������Ľ����㸴ԭ��
              Delete_unexchange_two_phase_point();  //��������ĵ�ɾ����
              main_black_point_number--;
              main_white_point_number--;
            }
          }
        }
        if (main_white_point_number == 0 || main_black_point_number == 0) {
          break;
        }
      }
      Delete_site_vector();  //�����ֽ������������գ��Ա���һ��ѭ����
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
    Set_reconstruct_to_inital();  //��reconstructed�е��м��ؽ����ת�Ƶ�inital
    Putout_step_image(m_dstimg_path);  //���ԭʼͼ���Ӧ����ͼ��
  }
  Set_reconstruct_to_final();
  Putout_final_image(m_dstimg_path);

  return true;
}

/////////////////////////////////1��ȷ����С����/////////////////////////////////////////
void CAnnealing::Decide_min_grid(int& min_grid) {
  int i = 0, j = 0, p = 0, q = 0, sum = 0, k = 0, w = 0;
  int white_point = 0, black_point = 0;
  int loop_time = log((float)m_initial_width) / log(2.0);
  int temporay_value_1 = 0, temporay_value_2 = 0;
  bool judgement = false;

  for (k = 0; k != loop_time - 1; k++)  ////��С�����4*4��ʼ
  {
    temporay_value_1 = (int)pow(2.0, k);  ////���������֮��ľ���
    temporay_value_2 = m_initial_width / temporay_value_1;  ////����Ĵ�С
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
        i++)  // 3*3ģ������ĵ㣬�жϷּ�ͼ���Ƿ����3*3����Ϊͬһ�ֻ࣬Ҫ���ڣ������Ѱ��
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
  //�ȶ���һ����ά���������ڴ�ų�ʼ��С����
  int i = 0, j = 0, k = 0, inital_white_point = 0;

  int new_inital_size = m_Rec_min_grid_size * m_multiple;  // 190113

  m_initial_rand_data.resize(
      new_inital_size);  // m_initial_rand_data��ʼ��С���������ֵ��
  for (i = 0; i != new_inital_size; ++i) {
    m_initial_rand_data[i].resize(new_inital_size);
    for (j = 0; j != new_inital_size; ++j) {
      m_initial_rand_data[i][j].resize(new_inital_size);
      for (k = 0; k != new_inital_size; k++) {
        m_initial_rand_data[i][j][k] = 2;
      }
    }
  }
  //���տ�϶�Ƚ����������ֵ����ɫ��1��ʾ����ɫ��0��ʾ
  int size_number = m_initial_height / m_Rec_min_grid_size;
  for (i = 0; i != m_Rec_min_grid_size; i++) {
    for (j = 0; j != m_Rec_min_grid_size; j++) {
      if (m_initial_imgvec[i * size_number][j * size_number] ==
          1)  // m_initial_imgvec���ԭͼ����
      {
        inital_white_point++;
      }
    }
  }
  int rand_site = 0;
  int counter_number =
      new_inital_size * new_inital_size *
      new_inital_size;  // 190113��ԭRec_min_grid_size*Rec_min_grid_size*Rec_min_grid_size
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

  //ԭͼ�Ŀ�϶�����൱�ڴ��ؽ���ά�ṹһ��Ŀ�϶���������Բ�����Ϊ������ά�ṹ��϶��
  for (i = 0;
       i != new_inital_size * inital_white_point * (m_multiple * m_multiple);
       i++)  //����Ҫ�����Σ�������ı�
  {
    rand_site = rand32() % counter_number;
    m_initial_rand_data[point_x[rand_site]][point_y[rand_site]]
                       [point_z[rand_site]] = 1;  // 1��ʾ��ɫ�㣬0��ʾ��ɫ��
    site_x = point_x.begin() +
             rand_site;  //�ҵ��Ѿ������ֵ�ĵ㣬��ɾ���������ظ���ֵ
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

// m_initial_rand_data(��ʼ��С����)��ʼ��(190113��)->(190212���������ά�ṹ��ϢҲ���浽��С������)
//��׿׵�����λ�ó�ʼ��Ϊ300������190212�����ؽ��ı���������ڴ�׿׵����ڵ�λ�ã��������ܱ�֤�ؽ�����Ŀ�϶����ȷ���ұ�֤��׽ṹ����Ӱ��
//��׿׵�λ��ֻ������ΪС�׿׵㲻���ؽ�����������С�ױ����㣨��׿׵�̶������ܱ������ֵ��
void CAnnealing::Set_inital_data_2fuse3() {
  //�ȶ���һ����ά���������ڴ�ų�ʼ��С����
  int i = 0, j = 0, k = 0, inital_white_point = 0;

  int new_inital_size = m_Rec_min_grid_size * m_multiple;  // 190113
  int size_number = m_initial_height / m_Rec_min_grid_size;
  int b_pnum = 0;  //��׿׵���                      //190212

  // m_initial_rand_data��ʼ��Ϊ2��300��֮ǰȫ��2��
  m_initial_rand_data.resize(
      new_inital_size);  // m_initial_rand_data��ʼ��С���������ֵ��
  for (i = 0; i != new_inital_size; ++i) {
    m_initial_rand_data[i].resize(new_inital_size);
    for (j = 0; j != new_inital_size; ++j) {
      m_initial_rand_data[i][j].resize(new_inital_size);
      for (k = 0; k != new_inital_size; k++) {
        if (m_big_3Dimgvec[i * size_number][j * size_number][k * size_number] !=
            0) {
          m_initial_rand_data[i][j][k] = 300;  // 190212����׿׵�����λ��
          ++b_pnum;
        } else
          m_initial_rand_data[i][j][k] = 2;
      }
    }
  }
  //���տ�϶�Ƚ����������ֵ����ɫ��1��ʾ����ɫ��0��ʾ
  for (i = 0; i != m_Rec_min_grid_size; i++) {
    for (j = 0; j != m_Rec_min_grid_size; j++) {
      if (m_initial_imgvec[i * size_number][j * size_number] ==
          1)  // m_initial_imgvec���ԭͼ�������߷ֱ棩
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
  //ԭͼ�Ŀ�϶�����൱�ڴ��ؽ���ά�ṹһ��Ŀ�϶���������Բ�����Ϊ������ά�ṹ��϶��
  for (i = 0;
       i != new_inital_size * inital_white_point * (m_multiple * m_multiple);
       i++) {
    rand_site = rand32() % counter_number;
    if (m_initial_rand_data[point_x[rand_site]][point_y[rand_site]]
                           [point_z[rand_site]] != 300)  // 190212
    {
      m_initial_rand_data[point_x[rand_site]][point_y[rand_site]]
                         [point_z[rand_site]] = 1;  // 1��ʾ��ɫ�㣬0��ʾ��ɫ��
      site_x = point_x.begin() +
               rand_site;  //�ҵ��Ѿ������ֵ�ĵ㣬��ɾ���������ظ���ֵ
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
          rand_site;  // 190213���ҵ�����׿׵㡱����λ�ã���ɾ���������ظ���ֵ
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

//Ϊ(new)m_vector_size(��ǰ��ѵ��ͼ��ĳߴ�)��ֵ
void CAnnealing::Transfer_grid_size(int T_grid_size) {
  m_gridsize = T_grid_size;
  m_vector_size = m_Rec_min_grid_size * (int)pow(2.0, m_gridsize);
  m_new_vector_size = m_vector_size * m_multiple;  // 190113
}

// TI_code_value(���ѵ��ͼ��Ķ���ܶȺ���)
void CAnnealing::TI_Multi_point_density_function_p() {
  m_train_code_value.clear();  // TI_code_value���ѵ��ͼ��Ķ���ܶȺ���
                               // //����һ����洢�Ķ���ܶȺ������
  int i = 0, j = 0;
  int reduce_size = m_initial_height / m_vector_size;  //��С����
  int k = m_Rec_template_size / 2;
  vector<vector<int>> Temporary_vector;
  Temporary_vector.resize(m_vector_size);  //���浱ǰ������TI
  for (i = 0; i != m_vector_size; ++i) {
    Temporary_vector[i].resize(m_vector_size);
  }

  //����Ӧ���ԭʼͼ��ŵ��м��������
  for (i = 0; i != m_vector_size; i++) {
    for (j = 0; j != m_vector_size; j++) {
      if (m_initial_imgvec[i * reduce_size][j * reduce_size] == 1) {
        Temporary_vector[i][j] = 1;  //Ϊ�˺�����뷽�㽫255ֵ��Ϊ1
      } else {
        Temporary_vector[i][j] = 0;
      }
    }
  }

  //�����ʱ��Сͼ��Ķ���ܶȺ���
  int code_value = 0;  //ģʽ����ֵ
  int q = 0, p = 0;
  for (i = 0; i != m_vector_size; ++i) {
    for (j = 0; j != m_vector_size; ++j) {
      code_value = 0;
      for (p = -k; p != k + 1; p++) {
        for (q = -k; q != k + 1; q++) {
          //���ֱ��뷽ʽ�ܹ���֤���벻�ظ���������ֵΪ511
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

//���ÿһ��ԭʼͼ���Ӧ����ͼ��
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
  sprintf(name, "The inital%d!.bmp", m_gridsize);  // grid_size����T_grid_size
  QString respath = filepath;
  respath.append("/initial/").append(name);
  img.save(respath, nullptr, 100);
}

// 190212��m_reconstruct_vec��һ�γ�ʼ��ʱ��׿׵�����λ�ö���ʼ��Ϊ0�ˣ�
void CAnnealing::
    Set_inital_to_reconstruct_uncondition_1()  //Ϊ��С�����Ӧ����ά�ṹ����ֵ����ΪSet_inital_data��
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
  /////////////////////////////////////////////////���о���������ֵ�����ϲ�ͼ�񴫵ݸ��ؽ�������
  m_reconstruct_vec.resize(m_new_vector_size);  //(190113)����ؽ��������
  m_preproc_vector.resize(
      m_new_vector_size);  //���Ԥ��������������ע3*3ȫΪ�ڻ���ȫ�׵�����
  for (i = 0; i != m_new_vector_size; ++i) {
    m_reconstruct_vec[i].resize(m_new_vector_size);
    m_preproc_vector[i].resize(m_new_vector_size);
    for (j = 0; j != m_new_vector_size; j++) {
      m_reconstruct_vec[i][j].resize(m_new_vector_size);
      m_preproc_vector[i][j].resize(m_new_vector_size);
      for (k = 0; k != m_new_vector_size; k++) {
        m_preproc_vector[i][j][k] = 2;   /////��ʼ���м�����Ϊ2
        m_reconstruct_vec[i][j][k] = 2;  /////��ʼ���м�����Ϊ2
      }
    }
  }

  for (i = 0; i != m_vector_size; ++i) {
    for (j = 0; j != m_vector_size; ++j) {
      if (m_initial_imgvec[i * distance_point][j * distance_point] ==
          1)  /////���㱾��ԭͼ��ɫ����
      {
        current_grid_white_number++;  //��һ��ģ�16��֮ǰ�Ѿ������ó��������
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
            [k];  /////���ϲ��ؽ��Ľ�����ݵ���Ӧ����㣬m_initial_rand_dataҲ�ǳ˹���������
        m_preproc_vector[2 * i][2 * j][2 * k] =
            m_initial_rand_data[i][j][k];  /////���ϲ��ؽ��Ľ�����ݵ���Ӧ�����
        if (m_initial_rand_data[i][j][k] == 1) {
          up_grid_white_number++;  /////�����ϲ��ؽ��Ľ����϶�����
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
            k % 2 == 1)  // 3*3ģ�����ĵ㳤������궼����Ϊ������
        {
          sum = 0;  /////�ж�3*3*3�����Ƿ�ȫΪ1����ȫ0
          for (q = -1; q != 2; q++)  // i,j,kΪ��������+��=ż����+ż=�棩
          {
            for (p = -1; p != 2; p++) {
              for (n = -1; n != 2; n++) {
                //�����������Ҫ���й�ʽ��1��Ϊż����(i+q+m_new_vector_size)%m_new_vector_sizeʹ�õĲ��������ڱ߽���������ֹԽ�磡
                if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((k + n + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0) {
                  // q��p��n��ҪΪ����
                  //������������(-1/1,-1/1,-1/1)��8���㣬���Ǹպ�Ϊǰһ��ӳ������ĵ㣨8���ǣ�
                  sum = sum +
                        m_reconstruct_vec
                            [(i + q + m_new_vector_size) % m_new_vector_size]
                            [(j + p + m_new_vector_size) % m_new_vector_size]
                            [(k + n + m_new_vector_size) % m_new_vector_size];
                }
              }
            }
          }
          //ע�⣺m_reconstruct_vec���ʼ��ֵΪ2������һ���ֵ㣨�����ֵ0��2��4...����ֵ�ָ���ǰһ���ؽ��������ʼ��Ϊ0��1
          //����sum��ͳ�Ƶ�ǰһ��ӳ����������ص㣡
          if (sum == 8)  /////ȫ1ʱ�������������ĵ��⣬�����ȫ��ֵΪ1
          {
            for (q = -1; q != 2; q++)  // i,j,kΪ��������+��=ż����+ż=�棩
            {
              for (p = -1; p != 2; p++) {
                for (n = -1; n != 2; n++) {
                  //��һ����ʽ�����Ϊ��������
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
  // current_grid_white_numberΪ����ѵ��ͼ���еİ׵���(190113)��up_grid_white_numberΪǰһ���ؽ�����еİ׵���
  all_add_white_number = m_new_vector_size * current_grid_white_number *
                             (m_multiple * m_multiple) -
                         up_grid_white_number;  /////����������Ҫ��Ӱ�ɫ�����
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

  for (i = 0; i != m_new_vector_size; ++i)  /////ͳ���������1��0�ĸ���
  {
    for (j = 0; j != m_new_vector_size; ++j) {
      for (k = 0; k != m_new_vector_size; ++k) {
        // m_preproc_vector��Ϊ�ڰ׵���ܴ��ڵ�λ��
        //������ľۺϹ��ɣ�ǰһ��ȫΪ�׵�/�ڵ��ģʽ������һ������չģʽ����չ������Ҳ�ǰ׵�/�ڵ�
        if (m_preproc_vector[i][j][k] == 31)  // 3*3*3����ȫΪ1
        {
          white_point_number_3D++;
        } else if (m_preproc_vector[i][j][k] == 30)  // 3*3*3����ȫΪ0
        {
          black_point_number_3D++;
        }
      }
    }
  }

  int point_x = 0;
  int point_y = 0;
  int point_z = 0;

  //��ʵ�ʴ���ӵĺڰ׵���>=���ݾۺϹ��ɼ���ĺڰ׵�������ֱ�ӽ�����ĺڰ׵�λ��ֱ�Ӱ����踳ֵ
  if (all_add_white_number >= white_point_number_3D &&
      all_add_black_number >=
          black_point_number_3D)  /////��m_reconstruct_vec��Ӧ3D����㸳ֵ1��0
  {
    for (i = 0; i != m_new_vector_size;
         ++i)  //�Ƚ�3*3*3�����жϵĵ�ֱ�ӽ���Щ�������������ݸ�ֵ
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

    ////////////////////////////////////////////////////////////////////////////���ϲ����Ǵ����ϼ�����3*3*3����Ϊȫ1��ȫ0���
    ////////////////////////////////////////////////////////////////////////////���²����Ǵ����ϼ�����2*2����Ϊȫ1��ȫ0���
    for (i = 0; i != m_new_vector_size; ++i)  //�ж�ȫ1����ȫ0��2*2*2��������
    {
      for (j = 0; j != m_new_vector_size; ++j) {
        for (k = 0; k != m_new_vector_size; ++k) {
          // iΪż����j��kΪ�����������ص㻹δ��ֵ�����ǳ�ʼֵ2��
          if (i % 2 == 0 && j % 2 == 1 && k % 2 == 1 &&
              m_preproc_vector[i][j][k] == 2) {
            sum = 0;
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                //ֻ����j��k����Ҫq��pΪ����
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

          // kΪż����j��iΪ�����������ص㻹δ��ֵ�����ǳ�ʼֵ2��
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

          // jΪż����k��iΪ�����������ص㻹δ��ֵ�����ǳ�ʼֵ2��
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
    /////�ڶ���2*2���ĵ�ֱ�Ӹ�ֵ�����϶��
    if (all_add_white_number >=
            (white_point_number_3D + white_point_number_2D) &&
        all_add_black_number >=
            (black_point_number_3D + black_point_number_2D)) {
      for (i = 0; i != m_new_vector_size; ++i)  //��ֱ�ӽ���Щ�������������ݸ�ֵ
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

      ///////////////�������ж�2*2�����Ƿ�ȫΪ0����1
      add_white_number_2D = (all_add_white_number - white_point_number_3D -
                             white_point_number_2D);  //��Ȼ���ʣ�ಿ�������ֵ
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

    else  /////�ڶ���2*2*2���ĵ�ֱ�Ӹ�ֵ�������϶��ʱ�����˵���һ��3*3*3����״̬
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

  //ʵ�ʴ���ӵĺڰ׵��� < ���ݾۺϹ��ɼ���ĺڰ׵�����3D�������Ķ��Ѿ����ˣ�
  else  /////ֱ�Ӱ�����Ҫ�����ֵ
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
                               [k];  /////���m_preproc_vector��Ϊָʾ����
        }
      }
    }
  }
  /////��m_preproc_vector����Ϊָʾ���������ϼ������ģ��ۺϹ��ɸ�ֵ�ģ�����Ϊ2
  for (i = 0; i != m_new_vector_size; i++) {
    for (j = 0; j != m_new_vector_size; j++) {
      for (k = 0; k != m_new_vector_size; k++) {
        if (m_preproc_vector[i][j][k] != 2 && m_reconstruct_vec[i][j][k] == 1) {
          m_preproc_vector[i][j][k] = 1;  //֮ǰ��31��21
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
  /////////////////////////////////////////////////���о���������ֵ�����ϲ�ͼ�񴫵ݸ��ؽ�������
  m_reconstruct_vec.resize(m_new_vector_size);  //(190113)����ؽ��������
  m_preproc_vector.resize(
      m_new_vector_size);  //���Ԥ��������������ע3*3ȫΪ�ڻ���ȫ�׵�����
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
          m_reconstruct_vec[i][j][k] = 2;  /////��ʼ���м�����"��׿׵�"Ϊ300
        } else {
          m_preproc_vector[i][j][k] = 2;   /////��ʼ���м�����Ϊ2
          m_reconstruct_vec[i][j][k] = 2;  /////��ʼ���м�����Ϊ2
        }
      }
    }
  }
  for (i = 0; i != m_vector_size; ++i) {
    for (j = 0; j != m_vector_size; ++j) {
      if (m_initial_imgvec[i * distance_point][j * distance_point] ==
          1)  /////���㱾��ԭͼ��ɫ����
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
                                 [k];  /////���ϲ��ؽ��Ľ�����ݵ���Ӧ�����
          m_preproc_vector[2 * i][2 * j][2 * k] =
              m_initial_rand_data[i][j]
                                 [k];  /////���ϲ��ؽ��Ľ�����ݵ���Ӧ�����
          if (m_initial_rand_data[i][j][k] == 1) {
            up_grid_white_number++;  /////�����ϲ��ؽ��Ľ����϶�ࣨ190212�������²��׿׵��غϣ��ĸ���
          }
        } else {
          // if (m_initial_rand_data[i][j][k]==0)
          // //190213����׿׵����ڵ�λ��ֵҲΪ0��ȥ����һ����������׿� {
          ////������λ����Ϊ��֪�ı��������㣬�Ա�ʵ��������
          m_reconstruct_vec[2 * i][2 * j][2 * k] = 0;
          /*}*/
        }
      }
    }
  }

  /////////////////////////////////////////////////////
  for (i = 0; i != m_new_vector_size;
       ++i)  // 190113����������x��y��z��Ӧ�����ص㣡��
  {
    for (j = 0; j != m_new_vector_size; ++j) {
      for (k = 0; k != m_new_vector_size; ++k) {
        if (i % 2 == 1 && j % 2 == 1 &&
            k % 2 == 1)  // 3*3ģ�����ĵ㳤������궼����Ϊ������
        {
          sum = 0;  /////�ж�3*3*3�����Ƿ�ȫΪ1����ȫ0
          for (q = -1; q != 2; q++)  // i,j,kΪ��������+��=ż����+ż=�棩
          {
            for (p = -1; p != 2; p++) {
              for (n = -1; n != 2; n++) {
                //�����������Ҫ���й�ʽ��1��Ϊż����(i+q+m_new_vector_size)%m_new_vector_sizeʹ�õĲ��������ڱ߽���������ֹԽ�磡
                if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0 &&
                    ((k + n + m_new_vector_size) % m_new_vector_size) % 2 ==
                        0) {
                  // q��p��n��ҪΪ����
                  //������������(-1/1,-1/1,-1/1)��8���㣬���Ǹպ�Ϊǰһ��ӳ������ĵ�
                  sum = sum +
                        m_reconstruct_vec
                            [(i + q + m_new_vector_size) % m_new_vector_size]
                            [(j + p + m_new_vector_size) % m_new_vector_size]
                            [(k + n + m_new_vector_size) % m_new_vector_size];
                }
              }
            }
          }
          //ע�⣺m_reconstruct_vec���ʼ��ֵΪ2������һ���ֵ㣨�����ֵ0��2��4...����ֵ�ָ���ǰһ���ؽ��������ʼ��Ϊ0��1
          //����sum��ͳ�Ƶ�ǰһ��ӳ����������ص㣡
          if (sum == 8)  /////ȫ1ʱ�������������ĵ��⣬�����ȫ��ֵΪ1
          {
            for (q = -1; q != 2; q++)  // i,j,kΪ��������+��=ż����+ż=�棩
            {
              for (p = -1; p != 2; p++) {
                for (n = -1; n != 2; n++) {
                  //��һ����ʽ�����Ϊ��������
                  if (((i + q + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1 ||
                      ((j + p + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1 ||
                      ((k + n + m_new_vector_size) % m_new_vector_size) % 2 ==
                          1) {
                    // q��p��n������һ��Ϊż������0��
                    //��������ǰһ��ӳ������ĵ�ȫΪ1ʱ��3*3*3ģ��������ֵΪ2�ĵ㣨��չ�㣩
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
          } else if (sum == 0)  /////ȫ0ʱ�������������ĵ��⣬�����ȫ��ֵΪ0
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
                    // //190212�����Բ�Ҫ��������ɣ���׿׵������С�׽ṹ�ı�����
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
  // current_grid_white_numberΪ����ѵ��ͼ���еİ׵���(190113)��up_grid_white_numberΪǰһ���ؽ�����еİ׵���
  all_add_white_number = m_new_vector_size * current_grid_white_number *
                             (m_multiple * m_multiple) -
                         up_grid_white_number;  /////����������Ҫ��Ӱ�ɫ�����
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

  for (i = 0; i != m_new_vector_size; ++i)  /////ͳ���������1��0�ĸ���
  {
    for (j = 0; j != m_new_vector_size; ++j) {
      for (k = 0; k != m_new_vector_size; ++k) {
        // m_preproc_vector��Ϊ�ڰ׵���ܴ��ڵ�λ��
        //������ľۺϹ��ɣ�ǰһ��ȫΪ�׵�/�ڵ��ģʽ������һ������չģʽ����չ������Ҳ�ǰ׵�/�ڵ�
        if (m_preproc_vector[i][j][k] == 31)  // 3*3*3����ȫΪ1
        {
          white_point_number_3D++;
        } else if (m_preproc_vector[i][j][k] == 30)  // 3*3*3����ȫΪ0
        {
          black_point_number_3D++;
        }
      }
    }
  }

  int point_x = 0;
  int point_y = 0;
  int point_z = 0;

  //��ʵ�ʴ���ӵĺڰ׵���>=���ݾۺϹ��ɼ���ĺڰ׵�������ֱ�ӽ�����ĺڰ׵�λ��ֱ�Ӱ����踳ֵ
  if (all_add_white_number >= white_point_number_3D &&
      all_add_black_number >=
          black_point_number_3D)  /////��m_reconstruct_vec��Ӧ3D����㸳ֵ1��0
  {
    for (i = 0; i != m_new_vector_size;
         ++i)  //�Ƚ�3*3*3�����жϵĵ�ֱ�ӽ���Щ�������������ݸ�ֵ
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
    //���棺����ż����
    ////////////////////////////////////////////////////////////////////////////���ϲ����Ǵ����ϼ�����3*3*3����Ϊȫ1��ȫ0���
    ////////////////////////////////////////////////////////////////////////////���²����Ǵ����ϼ�����2*2����Ϊȫ1��ȫ0���
    for (i = 0; i != m_new_vector_size; ++i)  //�ж�ȫ1����ȫ0��2*2*2��������
    {
      for (j = 0; j != m_new_vector_size; ++j) {
        for (k = 0; k != m_new_vector_size; ++k) {
          // iΪż����j��kΪ�����������ص㻹δ��ֵ�����ǳ�ʼֵ2||(190212��300)��
          if (i % 2 == 0 && j % 2 == 1 && k % 2 == 1 &&
              (m_preproc_vector[i][j][k] == 2 ||
               m_preproc_vector[i][j][k] == 300)) {
            sum = 0;
            for (q = -1; q != 2; q++) {
              for (p = -1; p != 2; p++) {
                //ֻ����j��k����Ҫq��pΪ����
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
                  300)  // 190213���Ƿ��е����⣿Pre������ڵ�λ�ÿ�������һ�����Ѿ���ֵΪ30����
              {  //û���⣡��Ϊ���������Pre��ֵֻ��Ϊ2��300��
                m_preproc_vector[i][j][k] = 21;
                white_point_number_2D++;
              }
            } else if (sum == 0) {
              m_preproc_vector[i][j][k] = 20;
              black_point_number_2D++;
            }
          }

          // kΪż����j��iΪ�����������ص㻹δ��ֵ�����ǳ�ʼֵ2||(190212��300)��
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

          // jΪż����k��iΪ�����������ص㻹δ��ֵ�����ǳ�ʼֵ2||(190212��300)��
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
    /////�ڶ���2*2���ĵ�ֱ�Ӹ�ֵ�����϶��
    if (all_add_white_number >=
            (white_point_number_3D + white_point_number_2D) &&
        all_add_black_number >=
            (black_point_number_3D + black_point_number_2D)) {
      for (i = 0; i != m_new_vector_size; ++i)  //��ֱ�ӽ���Щ�������������ݸ�ֵ
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

      ///////////////�������ж�2*2�����Ƿ�ȫΪ0����1
      add_white_number_2D = (all_add_white_number - white_point_number_3D -
                             white_point_number_2D);  //Ȼ���ʣ�ಿ�������ֵ
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

    else  /////�ڶ���2*2*2���ĵ�ֱ�Ӹ�ֵ�������϶��ʱ�����˵���һ��3*3*3����״̬
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

  else  /////ֱ�Ӱ�����Ҫ�����ֵ
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
                               [k];  /////���m_preproc_vector��Ϊָʾ����
        }
      }
    }
  }
  /////��m_preproc_vector����Ϊָʾ��
  for (i = 0; i != m_new_vector_size; i++) {
    for (j = 0; j != m_new_vector_size; j++) {
      for (k = 0; k != m_new_vector_size; k++) {
        if (m_preproc_vector[i][j][k] != 2 &&
            m_preproc_vector[i][j][k] != 300 &&
            m_reconstruct_vec[i][j][k] ==
                1) {  // 190213��������������Ҫ����ΪreconstΪ1��λ��һ�����Ǵ�׿׵�λ��
          m_preproc_vector[i][j][k] = 1;
        } else if (m_preproc_vector[i][j][k] != 2 &&
                   m_reconstruct_vec[i][j][k] == 0) {
          m_preproc_vector[i][j][k] = 0;
        }
      }
    }
  }
}

// Reconstruct_code_value(����ؽ�ͼ��Ķ���ܶȺ���)����190212��m_reconstruct_vec��û��300ֵ��
void CAnnealing::Recstruct_Multi_point_density_function_p() {
  m_reconstruct_code_value.clear();  //����һ����洢�Ķ���ܶȺ������
  int i = 0, j = 0, k = 0, q = 0, p = 0, r = 0;
  int s = m_Rec_template_size / 2;
  pair<map<int, int>::iterator, bool>
      pair_code_value;  //�����ʱ�ؽ�ͼ��Ķ���ܶȺ���
  int code_value = 0;   //ģʽ����ֵ
  for (k = 0; k != m_new_vector_size; ++k)  //���������ģʽ�ܶȽ���ͳ��
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

  //ע��,ԭ�����㷨�еļ�������,����
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
  //��ά�峤���Ӧ��һ��
  if (filelist.size() != m_final_imgsize) {
    QMessageBox msgBox;
    msgBox.setText(TR("������ͼ����Ӧ����ͼƬ�ߴ�,�����ļ���!"));
    msgBox.exec();
    return false;
  }

  //��������ͼ
  QImage img;
  m_big_3Dimgvec.resize(m_final_imgsize);
  int index = 0;
  for (auto itr = filelist.begin(); itr != filelist.end(); itr++, index++) {
    m_big_3Dimgvec[index].resize(m_final_imgsize);
    //���ļ�·����ӵ�m_filespath
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
  for (i = 0; i != m_sizenum; i++)  // size_numberģ���Ӧ��С�������ֵ
  {
    //(190113)3��ʾx��y��z�����棬m_vector_sizeΪ��ά������TI_code_valueΪ�����ģʽ�ܶȺ���
    E_differ = E_differ + abs(3 * m_new_vector_size * m_train_code_value[i] *
                                  (m_multiple * m_multiple) -
                              m_reconstruct_code_value[i]);
  }
  m_Enerry_origial = E_differ;
}

// white_point_number=Main_white_point_number
// 190213������m_reconstruct_vec��û�жԴ�׿׵�ı�־(300)�������޶���׿׵�λ��ֻ��Ϊ�������Ҳ��ܲ��뽻����
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
        if (m_reconstruct_vec[i][j][k] == 1)  //����ؽ��������
        {
          m_white_posx.push_back(i);
          m_white_posy.push_back(j);
          m_white_posz.push_back(k);
          ++white_point_number;
        } else if (
            m_big_3Dimgvec[i * distance_point][j * distance_point]
                          [k * distance_point] == 0 &&
            m_reconstruct_vec[i][j][k] ==
                0) {  // 190213��ֻ�зǴ�׿׵�����λ�õı�������ܲ��뽻����
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

  ///�жϽ��ϲ�������Ӧ��ǰ���������ֵ�෴���������Ϊ�����㣬�����ų��ϲ���Ϊȫ���ȫ1�����
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
             2 /*||m_preproc_vector[i][j][k]==300*/))  //���Ԥ��������������ע3*3ȫΪ�ڻ�ȫ�׵�����
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
            m_preproc_vector[i][j][k] = 1;  //��ɹ̶���
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
  // temporay_vector���ѡ�񽻻��㣨190212����׿׵��λ�ò��ܲ���������
  // Sign_exchange_vector��Ž����ɹ�������
  // m_preproc_vector����Ѿ�ȷ���ĵ㣬���ٲ��뽻��

  //////////////////////////////////////////////////////////ѡȡ������
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
// temporay_vector���ѡ�񽻻���
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

  ///�жϽ��ϲ�������Ӧ��ǰ���������ֵ�෴���������Ϊ�����㣬�����ų��ϲ���Ϊȫ���ȫ1�����
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
             2 /*||m_preproc_vector[i][j][k]==300*/))  //���Ԥ��������������ע3*3ȫΪ�ڻ�ȫ�׵�����
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
              m_preproc_vector[i][j][k] = 1;  //��ɹ̶���
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
                  300)  // 190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
                        // //�⿴��һ�����ã���Щ300���20/30��ѽ������ǰ��if�޶���preΪ2/300
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
                  300)  // 190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
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
                  300)  // 190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
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
                  300)  // 190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
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
                  300)  // 190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
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
                  300)  // 190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
              {
                temporay_vector[i][j][k] = 0;
              }
            }
          }
        }
      }
    }
  }
  // temporay_vector���ѡ�񽻻��㣨190212����׿׵��λ�ò��ܲ���������
  // Sign_exchange_vector��Ž����ɹ�������
  // m_preproc_vector����Ѿ�ȷ���ĵ㣬���ٲ��뽻��

  //////////////////////////////////////////////////////////ѡȡ������
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

//���������ԭ����
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

// white/black_rand_site ��/��ɫ��������λ��
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
  int code_value = 0;  //ģʽ����ֵ
  int q = 0, p = 0;
  int k = m_Rec_template_size / 2;
  int white_x = m_white_posx[m_white_randsite];
  int white_y = m_white_posy[m_white_randsite];
  int white_z = m_white_posz[m_white_randsite];
  int black_x = m_black_posx[m_black_randsite];
  int black_y = m_black_posy[m_black_randsite];
  int black_z = m_black_posz[m_black_randsite];
  pair<map<int, int>::iterator, bool> pair_code_value;

  //�������Ƚ�Exchange_code_value_white/black����ˣ�������ִ�������--����ʱ��Ϊʲô�������
  m_exchange_code_value_white.clear();
  m_exchange_code_value_black.clear();
  for (q = -k; q != k + 1; q++)  ////��ѭ��Ϊÿ�������������9��ģʽ
  {
    for (p = -k; p != k + 1; p++) {
      m_reconstruct_vec[white_x][white_y][white_z] =
          1;  //����ǰ(190212����������temporay_vector���ѡ��Ľ�����ȷ����)
      m_reconstruct_vec[black_x][black_y][black_z] = 0;

      ////////////////////////////////////////////////////XYƽ�潻���������ģʽ

      code_value = 0;
      for (i = -k; i != k + 1; i++)  ////��ѭ��Ϊ����ÿ��ģʽ
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
          [code_value];  //��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ

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
            [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
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
            [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
      }

      ////////////////////////////////////////////////////XZƽ�潻���������ģʽ
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
          [code_value];  //��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ

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
            [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
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
            [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
      }

      //////////////////////////////////////////////////YZƽ�潻���������ģʽ
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
          [code_value];  //��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ

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
            [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
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
            [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      m_reconstruct_vec[white_x][white_y][white_z] = 0;  //������
      m_reconstruct_vec[black_x][black_y][black_z] = 1;
      ////////////////////////////////////////////////////XYƽ�潻���������ģʽ
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
          make_pair(code_value, 1));  //���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
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
        if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
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
        if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
        {
          ++pair_code_value.first->second;
        }
      }

      ////////////////////////////////////////////////////YZƽ�潻���������ģʽ
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
          make_pair(code_value, 1));  //���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
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
        if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
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
        if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
        {
          ++pair_code_value.first->second;
        }
      }

      ////////////////////////////////////////////////////XZƽ�潻���������ģʽ
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
          make_pair(code_value, 1));  //���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
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
        if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
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
        if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
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
                (m_multiple * m_multiple));  //����û�б仯��ģʽ����
    m_reconstruct_code_value[iter_Success->first] =
        m_reconstruct_code_value[iter_Success->first] +
        m_exchange_code_value_white[iter_Success->first];  //����ԭ����ģʽ
    Engerry_after_exchange_white =
        Engerry_after_exchange_white +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_new_vector_size * m_train_code_value[iter_Success->first] *
                (m_multiple * m_multiple));  //����ģʽ��ֵ
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
                (m_multiple * m_multiple));  //����û�б仯��ģʽ����
    m_reconstruct_code_value[iter_Success->first] =
        m_reconstruct_code_value[iter_Success->first] +
        m_exchange_code_value_black[iter_Success->first];  //����ԭ����ģʽ
    Engerry_after_exchange_black =
        Engerry_after_exchange_black +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_new_vector_size * m_train_code_value[iter_Success->first] *
                (m_multiple * m_multiple));  //����ģʽ��ֵ
  }
  Engerry_change_black =
      Engerry_after_exchange_black - Engerry_before_exchange_black;

  m_Enerry_Reverse = m_Enerry_origial;  //������ǰ��������棬�Ա��滻����
  Engerry_down = m_Enerry_origial + Engerry_change_white +
                 Engerry_change_black;  //���ݽ������������
  m_Enerry_origial = Engerry_down;      //��������������Ϊ��״̬
  m_reconstruct_vec[white_x][white_y][white_z] = 0;  //������
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
        if (m_reconstruct_vec[i][j][k] == 1)  //����ؽ��������
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
    //����ͼƬ����ʽΪ���ֱ�ţ�����1.bmp��
    QString respath = filepath;
    respath.append("/reconstruct/").append(name);
    img.save(respath, nullptr, 100);
  }
}

// 190213�����������׵���ά��϶�ṹ
void CAnnealing::Set_reconstruct_to_final() {
  int i = 0, j = 0, k = 0;
  if (m_final_imgsize != m_new_vector_size) {
    cout << "�ؽ��ߴ����" << endl;
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
  m_train_code_value.clear();  //����һ����洢�Ķ���ܶȺ������
  int i = 0, j = 0;
  int reduce_size = m_initial_height / m_vector_size;  //��С����
  int k = m_Rec_template_size / 2;
  vector<vector<int>> Temporary_vector;
  Temporary_vector.resize(m_vector_size);
  for (i = 0; i != m_vector_size; ++i) {
    Temporary_vector[i].resize(m_vector_size);
  }

  //����Ӧ���ԭʼͼ��ŵ��м��������
  for (i = 0; i != m_vector_size; i++) {
    for (j = 0; j != m_vector_size; j++) {
      if (m_initial_imgvec[i * reduce_size][j * reduce_size] == 1) {
        Temporary_vector[i][j] = 1;  //Ϊ�˺�����뷽�㽫255ֵ��Ϊ1
      } else {
        Temporary_vector[i][j] = 0;
      }
    }
  }

  //�����ʱ��Сͼ��Ķ���ܶȺ���
  int code_value = 0;  //ģʽ����ֵ
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
  m_reconstruct_code_value.clear();  //����һ����洢�Ķ���ܶȺ������
  int i = 0, j = 0, k = 0, q = 0, p = 0, r = 0;
  int s = m_Rec_template_size / 2;
  pair<map<int, int>::iterator, bool>
      pair_code_value;  //�����ʱ�ؽ�ͼ��Ķ���ܶȺ���
  int code_value = 0;   //ģʽ����ֵ
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
  int code_value = 0;  //ģʽ����ֵ
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
      m_reconstruct_vec[white_x][white_y][white_z] = 1;  //����ǰ
      m_reconstruct_vec[black_x][black_y][black_z] = 0;
      ////////////////////////////////////////////////////XYƽ�潻���������ģʽ
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
            [code_value];  //��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ
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
              [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
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
              [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
        }
      }
      ////////////////////////////////////////////////////XZƽ�潻���������ģʽ
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
            [code_value];  //��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ
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
              [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
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
              [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
        }
      }
      //////////////////////////////////////////////////YZƽ�潻���������ģʽ
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
            [code_value];  //��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ
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
              [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
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
              [code_value];  //��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
        }
      }
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      m_reconstruct_vec[white_x][white_y][white_z] = 0;  //������
      m_reconstruct_vec[black_x][black_y][black_z] = 1;
      ////////////////////////////////////////////////////XYƽ�潻���������ģʽ
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
            make_pair(code_value, 1));  //���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
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
          if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
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
          if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
          {
            ++pair_code_value.first->second;
          }
        }
      }

      ////////////////////////////////////////////////////YZƽ�潻���������ģʽ
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
            make_pair(code_value, 1));  //���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
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
          if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
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
          if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
          {
            ++pair_code_value.first->second;
          }
        }
      }

      ////////////////////////////////////////////////////XZƽ�潻���������ģʽ
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
            make_pair(code_value, 1));  //���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
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
          if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
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
          if (!pair_code_value.second)  //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
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
                                       ->first]);  //����û�б仯��ģʽ����
    m_reconstruct_code_value[iter_Success->first] =
        m_reconstruct_code_value[iter_Success->first] +
        m_exchange_code_value_white[iter_Success->first];  //����ԭ����ģʽ
    Engerry_after_exchange_white =
        Engerry_after_exchange_white +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_vector_size *
                m_train_code_value[iter_Success->first]);  //����ģʽ��ֵ
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
                                       ->first]);  //����û�б仯��ģʽ����
    m_reconstruct_code_value[iter_Success->first] =
        m_reconstruct_code_value[iter_Success->first] +
        m_exchange_code_value_black[iter_Success->first];  //����ԭ����ģʽ
    Engerry_after_exchange_black =
        Engerry_after_exchange_black +
        abs(m_reconstruct_code_value[iter_Success->first] -
            3 * m_vector_size *
                m_train_code_value[iter_Success->first]);  //����ģʽ��ֵ
  }
  Engerry_change_black =
      Engerry_after_exchange_black - Engerry_before_exchange_black;

  m_Enerry_Reverse = m_Enerry_origial;  //������ǰ��������棬�Ա��滻����
  Engerry_down = m_Enerry_origial + Engerry_change_white +
                 Engerry_change_black;  //���ݽ������������
  m_Enerry_origial = Engerry_down;      //��������������Ϊ��״̬
  m_reconstruct_vec[white_x][white_y][white_z] = 0;  //������
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

  ///�жϽ��ϲ�������Ӧ��ǰ���������ֵ�෴���������Ϊ�����㣬�����ų��ϲ���Ϊȫ���ȫ1�����
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

  //////////////////////////////////////////////////////////ѡȡ������
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
  /////////////////////////////////////////////////���о���������ֵ�����ϲ�ͼ�񴫵ݸ��ؽ�������
  m_reconstruct_vec.resize(m_vector_size);
  for (i = 0; i != m_vector_size; ++i) {
    m_reconstruct_vec[i].resize(m_vector_size);
    for (j = 0; j != m_vector_size; ++j) {
      m_reconstruct_vec[i][j].resize(m_vector_size);
      for (k = 0; k != m_vector_size; k++) {
        if (i % 2 == 0 && j % 2 == 0 && k % 2 == 0) {
          m_reconstruct_vec[i][j][k] =
              m_initial_rand_data[i / 2][j / 2]
                                 [k / 2];  /////���ϲ��ؽ��Ľ�����ݵ���Ӧ�����
          if (m_initial_rand_data[i / 2][j / 2][k / 2] == 1) {
            up_white_number++;
          }
        } else {
          m_reconstruct_vec[i][j][k] = 2;  /////����㸳��ֵ2
        }
      }
    }
  }

  for (i = 0; i != m_vector_size; ++i) {
    for (j = 0; j != m_vector_size; ++j) {
      if (m_initial_imgvec[i * distance_point][j * distance_point] ==
          1)  /////ͳ�ƶ�Ӧ�����ɫ����
      {
        inital_white_number++;
      }
    }
  }
  add_white_number = m_vector_size * inital_white_number - up_white_number;
  int point_x = 0;
  int point_y = 0;
  int point_z = 0;
  /////////////////////////////////////////////////////////���ؽ���������λ�ð��տ�϶�������ֵ��
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
//	///����������ѡ�㷨�����ų��ϲ���Ϊȫ���ȫ1�����
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
//	///ѡȡ�������ĺ�ɫ���ص�
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
//	//////////////////////////////////////////////////////////ѡȡ������
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

  ///�жϽ��ϲ�������Ӧ��ǰ���������ֵ�෴���������Ϊ�����㣬�����ų��ϲ���Ϊȫ���ȫ1�����
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

  ///ѡȡ�������ĺ�ɫ���ص�
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
  // temporay_vector���ѡ�񽻻���
  // Sign_exchange_vector��Ž����ɹ�������
  // m_preproc_vector����Ѿ�ȷ���ĵ㣬���ٲ��뽻��

  //////////////////////////////////////////////////////////ѡȡ������
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

