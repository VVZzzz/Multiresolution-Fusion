#pragma once
#include <QString>
#include <bitset>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <stack>
#include <QObject>
using namespace std;
//��ά�ں���άͷ�ļ�
class PoreSet : public QObject{
  Q_OBJECT
 public:
  PoreSet(int porenum = 1, int templesz = 3);
  ~PoreSet();

 public:
  //void SetSmallSize(int layers, int height, int width);

 

  //�����ؽ��������Է���һ�������߳��н���
  //step0: �����ںϵ�С����ά�����,���·��
  void SetNumSize(int porenum, int templesz) {
    m_PoreNum = porenum;
    m_templesz = templesz;
  }
  void SetBigSize(int layers, int height, int width);
  void SetExpectPorosity(double p) { tar_p = p; }
  //step1: ������(�õ�ԭʼ��϶��)
  bool LoadBigPoreSet(const QString& filepath);
  //step2: ����С��
  bool LoadSmallPoreSet(vector<QString> &filepath);
  //step3: ����������϶��(SetExpectPorosity)
  //step4: �����ؽ�
  bool Reconstruct(const QString &savepath);

signals:
  //���ش�׽���
  void LoadBigPorePro(int, double);
  //�ؽ����ؽ���
  void SetProcessVal(int);

 private:

  bool Built3DImage01sPoreSet(const QString& filepath,int index,int total);
  void ClearSmallImg();
  void ClearBigImg();
  void ClearBigPorelabel();
  //bool ReconstructSpores(const QString& filepath, const QString& savepath);
  bool Is_ROI_B(int*** pore_labels, int*** Image3D, vector<bool>& ROI,
                int z_layers, int y_rows, int x_cols, int ROI_size,
                int _blayers, int _bRows, int _bCols, int flag01);
  bool IsReconstSuccess(const QString& savepath, double _chgnum,
                        double trag_num, int dev, int*** _3D, int _blayer,
                        int _bRows, int _bCols);

  bool _6direc_Reconst_Pore(int fillz, int a, int filly, int b, int fillx,
                            int c, int trans, int*** _3D, int*** _porelabel,
                            int _newlabel, double& _chgnum, int& _newporesize,
                            int _blayer, int _bRows, int _bCols, int& nflag);

  bool IsfindBestModel(vector<bool>& bs_newmodel, int& cur_modelpnum,
                       int new_modelpnum, int& min_diff, int& p_max, int& n,
                       stack<int>& suit_pnum, set<int> const& already_deal,
                       vector<bool>& best_m);


 private:
  int m_smalllayers;  //С�ײ���
  int m_smallwidth;   //С��ͼƬ�ֱ���
  int m_smallheight;
  int*** m_smallimg;  //����С����ά��

  int m_biglayers;
  int m_bigwidth;
  int m_bigheight;
  int*** m_bigimg;  //��������ά��

  int*** bPore_label;
  int b_porePnum;  //�����ά�ṹԭʼ�׵���
  double tar_p;   //�ؽ��������õ��Ŀ�϶��
  double initial_p;  //ԭʼ��׿�϶��

  int m_PoreNum;   //С����ά�ṹ����
  int m_templesz;  //ģʽ����ģ��ߴ�
  set<int> m_st_pnum;
  set<string> m_strkey;

  //����bitset<N>��N����������ֵ����,����������ʹ��vector<bool>���.
  // vector<bool>����򵥵���vec����
  multimap<int, pair<int, vector<bool>>> m_Map;
};
