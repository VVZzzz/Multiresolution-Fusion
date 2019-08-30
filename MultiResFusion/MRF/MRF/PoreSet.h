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
//三维融合三维头文件
class PoreSet : public QObject{
  Q_OBJECT
 public:
  PoreSet(int porenum = 1, int templesz = 3);
  ~PoreSet();

 public:
  //void SetSmallSize(int layers, int height, int width);

 

  //以下重建操作可以放在一个工作线程中进行
  //step0: 设置融合的小孔三维体个数,存放路径
  void SetNumSize(int porenum, int templesz) {
    m_PoreNum = porenum;
    m_templesz = templesz;
  }
  void SetBigSize(int layers, int height, int width);
  void SetExpectPorosity(double p) { tar_p = p; }
  //step1: 保存大孔(得到原始孔隙度)
  bool LoadBigPoreSet(const QString& filepath);
  //step2: 保存小孔
  bool LoadSmallPoreSet(vector<QString> &filepath);
  //step3: 设置期望孔隙度(SetExpectPorosity)
  //step4: 进行重建
  bool Reconstruct(const QString &savepath);

signals:
  //加载大孔进度
  void LoadBigPorePro(int, double);
  //重建加载进度
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
  int m_smalllayers;  //小孔层数
  int m_smallwidth;   //小孔图片分辨率
  int m_smallheight;
  int*** m_smallimg;  //保存小孔三维体

  int m_biglayers;
  int m_bigwidth;
  int m_bigheight;
  int*** m_bigimg;  //保存大孔三维体

  int*** bPore_label;
  int b_porePnum;  //大孔三维结构原始孔点数
  double tar_p;   //重建后期望得到的孔隙度
  double initial_p;  //原始大孔孔隙度

  int m_PoreNum;   //小孔三维结构个数
  int m_templesz;  //模式集中模块尺寸
  set<int> m_st_pnum;
  set<string> m_strkey;

  //由于bitset<N>中N必须是字面值常量,故我们这里使用vector<bool>替代.
  // vector<bool>不简简单单是vec容器
  multimap<int, pair<int, vector<bool>>> m_Map;
};
