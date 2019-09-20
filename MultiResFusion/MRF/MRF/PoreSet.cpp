#include "PoreSet.h"
#include <QFileInfoList>
#include <QDir>
#include <QImage>
#include <stack>
#include <QDebug>

PoreSet::PoreSet(int porenum, int templesz)
    : m_PoreNum(porenum), m_templesz(templesz),QObject() {
  m_smallimg = nullptr;
  m_bigimg = nullptr;
  bPore_label = nullptr;
  b_porePnum = 0;
  tar_p = 0;
  initial_p = 0;
  m_shutdown = false;
}

PoreSet::~PoreSet() {
  if (m_smallimg) ClearSmallImg();
  if (m_bigimg) ClearBigImg();
  if (bPore_label) ClearBigPorelabel();
}

/*
void PoreSet::SetSmallSize(int layers, int height, int width) {
  //m_smalllayers = layers;
  m_smallheight = height;
  m_smallwidth = width;
}
*/

void PoreSet::SetBigSize(int layers, int height, int width) {
  m_biglayers = layers;
  m_bigheight = height;
  m_bigwidth = width;
}

bool PoreSet::LoadSmallPoreSet(vector<QString>& filepath) {
  int i = 0, total = filepath.size();
  for (auto& str : filepath) {
    QImage img(str);
    int h = img.height();
    int w = img.width();
    m_smallheight = h;
    m_smallwidth = w;
    if (!Built3DImage01sPoreSet(str,i,total)) return false;
    i++;
    emit SetProcessVal(30*i/total);
  }
  return true;
}

//保存三维大孔结构
bool PoreSet::LoadBigPoreSet(const QString& filepath) { 
	/////////////////////////////////保存大孔三维结构/////////////////////////////////////////
  if (filepath.isEmpty()) return false;
  ClearBigImg();
  m_bigimg = new int**[m_biglayers];
  for (int i = 0; i < m_biglayers; i++) {
    m_bigimg[i] = new int*[m_bigheight];
    for (int j = 0; j < m_bigheight; j++) {
      m_bigimg[i][j] = new int[m_bigwidth]();
    }
  }

    QFileInfoList fileinfolist;
    QDir qdir(QFileInfo(filepath).absoluteDir());
    QStringList filters;
    filters << "*.png"
            << "*.jpg"
            << "*.bmp"
            << "*.jpeg";
    qdir.setNameFilters(filters);
    fileinfolist = qdir.entryInfoList();
    if (fileinfolist.size() != m_biglayers) return false;

    emit LoadBigPorePro(0, 0);
    QImage img;
    int index = 0;
    for (auto itr = fileinfolist.begin(); itr != fileinfolist.end(); itr++,index++) {
      emit LoadBigPorePro(index * 50 / m_biglayers, 0);
      img.load(itr->absoluteFilePath());
      QImage grayimg = img.convertToFormat(QImage::Format_Grayscale8);
      for (int i = 0; i < m_bigheight; i++) {
        const uchar* lpix = grayimg.scanLine(i);
        for (int j = 0; j < m_bigwidth; j++) {
          int pix = *(lpix + j * 1);
          m_bigimg[index][i][j] = (pix ? 255 : 0);
        }
      }
    }
    emit LoadBigPorePro(50, 0);

	double b_totalNum=m_biglayers*m_bigheight*m_bigwidth;   //三维大孔结构总像素点个数
	b_porePnum=0;    //大孔三维结构原始孔点数


	///////////////////////////////////为大孔三维结构中的孔设定孔点标签///////////////////////////////////////

  //将初始大孔三维结构中的孔点设定标签，判断重建小孔是否与大孔连通
  ClearBigPorelabel();
  bPore_label = new int**[m_biglayers];  
  for (int i = 0; i < m_biglayers; i++) {
    bPore_label[i] = new int*[m_bigheight];
    for (int j = 0; j < m_bigheight; j++) {
      bPore_label[i][j] = new int[m_bigwidth]();
    }
  }
	for (int z=0;z<m_biglayers;z++)
	{
    emit LoadBigPorePro(49 + z * 50 / m_biglayers, 0);
		for (int y=0;y<m_bigheight;y++)
		{
			for (int x=0;x<m_bigwidth;x++)
			{
				if (m_bigimg[z][y][x]!=0)   //孔点
				{
					bPore_label[z][y][x]=1;   //初始大孔三维结构中的孔点标签设为1
					b_porePnum++;
				}
				else
					bPore_label[z][y][x]=0;
			}
		}
	}
	//cout<<"大孔三维结构孔隙标签初始化完成！用时"<<time2<<"(min)"<<endl;
	//重建匹配思路：
	//根据填充的小孔模式块，作为该重建小孔的硬数据，再通过平移模板，获取新的待重建模式，在模式集中进行搜索匹配来重建该小孔剩余的孔点
	////////////////////////////////获取重建前和期望重建后的孔隙度(孔点个数)//////////////////////////////////////////
	//三维孔隙度(原始&期望值)
  /*
	double ori_p=(100*b_porePnum)/b_totalNum; 
	CString cs_ori_p;
	cs_ori_p.Format(_T("%.2f"),ori_p);
	cout<<"大孔三维结构的初始孔隙度(百分数)为："<<cs_ori_p<<endl;

	cout<<"请输入期望重建小孔后的三维结构孔隙度(百分数)(请保留两位小数)：";
	double tar_p;
	cin>>tar_p;
  */
  initial_p = (100.0 * b_porePnum) / b_totalNum;
  emit LoadBigPorePro(100, initial_p);
  return true;
}

bool PoreSet::Reconstruct(const QString& savepath) {  
  int b_totalNum = m_biglayers * m_bigheight * m_bigwidth;
	double tar_pNum=b_totalNum*(tar_p/100.0);
	double chg_pNum=b_porePnum;        //统计重建过程中的孔点数(变化值)
	int deviation=10;      //允许的偏差值

	int dev=(tar_pNum-b_porePnum)*0.65;     //12-15：前期按照之前的孔重建停止条件重建的孔点数

	//////////////////////////////////开始重建////////////////////////////////////////
	int new_poreNum=0;        //统计重建孔的个数
	while(chg_pNum<(tar_pNum-deviation))    //判断是否继续重建小孔，当最终图像孔隙点数小于指定最小孔隙点数，可继续重建满足条件的小孔
	{   
    emit SetProcessVal(30 + 69.0 * chg_pNum / (tar_pNum - deviation));
    if (m_shutdown) return false;


		/////////////////////////////////孔重建及判断重建完成的思路/////////////////////////////////////////

		//向上下前后左右，平移模板，从而得到6个新模块(每次平移一个像素点)，为新模块从模式集中寻找最匹配的模块（相同相位点个数必须达到设定的阈值），将匹配的模块替换新模块
		//若在重建过程中，某一方向接触到已重建的孔，（在重建需要改变已有孔点相位的情况下）保持已重建孔点不变。当重建模板中已重建其他孔点占一半及以上时，停止在该方向的重建（两孔已连通）

		int tran=0;      //平移量

		//重建当前小孔:
		//重建的某方向与其他孔相连或平移后新模块全为0，该方向不再重建；当连续随机6个方向都不能重建时，停止该孔的重建！

		int a1,b1,c1;         //a、b、c分别为确定重建方向的z、y、x的系数
		srand((unsigned)time(NULL));
		int low=-1;
		int high=1;     //产生-1~1之间的随机数，随机确定重建方向

		bool b_up=true, b_down=true, b_front=true, b_back=true, b_left=true, b_right=true;
		int nflag1=-1,nflag2=-1,nflag3=-1,nflag4=-1,nflag5=-1,nflag6=-1;

		//12-15：当重建的孔隙度在前后孔隙度差值的10%之内时，按照之前的重建方法
		//       当在10%之外时，采用新的重建结束判断，只有当当前重建孔与其他重建孔连接时才结束，否则将平移分量变为0，重新从头开始重建
		int flag01;      

		if (chg_pNum<(b_porePnum+dev))           
		{
			flag01=1;          

			//////////////////选取初始模式：首先随机确定子模式集，再在对应子模式集中随机选取初始模式块///////////////////////////////////
			srand((unsigned)time(NULL));
			int st_totalnum=m_st_pnum.size();
			//18-1-1修改：首先随机选取子模式集！（0~st_num-1）
			int r_st=rand()%(st_totalnum);         
			set<int>::const_iterator st_iter=m_st_pnum.begin();
			for (int i=0;i<r_st;i++)
			{
				++st_iter;
			}
			int c_modelpnum=*st_iter;      //当前选中的子模式集对应孔点数
			//18-1-1修改：再随机选取对应子模式集中模式块！（0~m-1）
			int m1=m_Map.count(c_modelpnum);
			int r_model=rand()%(m1);     
			multimap<int,pair<int,vector<bool> >>::const_iterator m_iter;
      

			m_iter=m_Map.equal_range(c_modelpnum).first;
			for (int i=0;i<r_model;i++)
			{
				++m_iter;
			}
			//bitset<bs_bits> ori_curModel=m_iter->second.second;
        vector<bool> ori_curModel = m_iter->second.second;

			//产生原三维结构待填充左上角坐标，判断位置是否能够填孔块(待填孔隙点位置全不为孔)
			//注意：控制坐标范围，不越界
			int fill_z=rand()%(m_biglayers-m_templesz+1);          //z坐标范围(0~b_layer-TempSize)
			int fill_y=rand()%(m_bigheight-m_templesz+1);           //y坐标范围(0~b_rows-TempSize)
			int fill_x=rand()%(m_bigwidth-m_templesz+1);           //x坐标范围(0~b_cols-TempSize)
      //选取位置不能填选取的小孔模块
      while (!Is_ROI_B(bPore_label, m_bigimg, ori_curModel,
                       fill_z, fill_y, fill_x, m_templesz,
                       m_biglayers, m_bigheight, m_bigwidth,
                       flag01)) {
        fill_z = rand() % (m_biglayers - m_templesz + 1);
        fill_y = rand() % (m_bigheight - m_templesz + 1);
        fill_x = rand() % (m_bigwidth - m_templesz + 1);
      }
			int a=2;
			int b=255;
			int new_label=rand()%(b-a+1)+a;     //为当前重建的小孔生成2~255的随机标签(0为图像背景，1为初始大孔标签)

			//填充所选的小模块到满足的位置，并同时改变当前三维结构的孔点数
			int m=-1;
			int new_poresize=0;     //统计新重建的小孔体积
			for (int z=fill_z;z<(fill_z+m_templesz);z++)
			{
				for (int y=fill_y;y<(fill_y+m_templesz);y++)
				{
					for (int x=fill_x;x<(fill_x+m_templesz);x++)
					{
						++m;
						//180312修正
						int t=0;
						//if (ori_curModel[m]!=0)
						if (ori_curModel[m])
						{
							t=255;
							m_bigimg[z][y][x]=t;
							chg_pNum++;     //整个三维结构总孔点数增加
							new_poresize++;
							bPore_label[z][y][x]=new_label;     //新增的小孔孔点标签
						}
					}
				}
			}

			while(b_up==true||b_down==true||b_left==true||b_right==true||b_front==true||b_back==true)      //某一方向仍能重建
			{
				tran++;

				/////////////////////////////////////判断整个重建过程是否停止/////////////////////////////////////
				if (IsReconstSuccess(savepath , chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					new_poreNum++; 
					//cout<<"已重建最后一个孔，为第"<<new_poreNum<<"个孔，该孔的体积为"<<new_poresize<<endl;
					return true;
				}
				
				if (b_up)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_up = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag1);
				}


				/////////////////////////////////////判断整个重建过程是否停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				
				if (b_down)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_down = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag2);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				
				if (b_left)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_left = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag3);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//右
				if (b_right)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_right = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag4);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//前
				if (b_front)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_front = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag5);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//后
				if (b_back)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_back = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag6);
				}

			}

			new_poreNum++; 

			//cout<<"已重建第"<<new_poreNum<<"个孔，该孔的体积为"<<new_poresize<<"，用时(min)"<<t<<endl;


			//////////////////////////////////不断更新&保存重建过程中的三维序列图////////////////////////////////////////
      //qDebug() << "更新三维结构文件夹";
      
      /*
      QDir qdir(savepath);
      if (!qdir.exists("reconstruct")) qdir.mkdir("reconstruct");
      QImage img(m_bigwidth, m_bigheight, QImage::Format_Grayscale8);
      int i = 0, j = 0, k = 0, q = 0, white_point = 0;
      for (q = 0; q != m_biglayers; q++) {
        for (i = 0; i != m_bigheight; ++i) {
          uchar* lpix = img.scanLine(i);
          for (j = 0; j != m_bigwidth; ++j) {
            k = m_bigimg[q][i][j];
            *(lpix + j * 1) = k ;
          }
        }
        char name[100];
        sprintf(name, "%d.bmp", q);
        //最终图片名格式为数字编号，例“1.bmp”
        QString respath = savepath;
        respath.append("/reconstruct/").append(name);
        img.save(respath, nullptr, 100);
      }
      */
		}
		else      //12-15：采用迭代重建的方法！只有当某一方向与其他孔连接才停止该孔的重建，否则将坐标偏移量置为0，从头开始重建
		{
			flag01=0;         //12-26：对于后百分之50的孔点，其放置位置必须可与任意孔靠近！ 

			//18-1-1修改：初始孔块随机确定子模式集，再在对应子模式集中随机选取初始模式块
			srand((unsigned)time(NULL));
			int st_totalnum=m_st_pnum.size();
			//18-1-1修改：首先随机选取子模式集！（0~st_num-1）
			int r_st=rand()%(st_totalnum);         
			set<int>::const_iterator st_iter=m_st_pnum.begin();
			for (int i=0;i<r_st;i++)
			{
				++st_iter;
			}
			int c_modelpnum=*st_iter;      //当前选中的子模式集对应孔点数
			//18-1-1修改：再随机选取对应子模式集中模式块！（0~m-1）
			int m1=m_Map.count(c_modelpnum);
			int r_model=rand()%(m1);     
			multimap<int,pair<int,vector<bool>>>::const_iterator m_iter;
			m_iter=m_Map.equal_range(c_modelpnum).first;

			for (int i=0;i<r_model;i++)
			{
				++m_iter;
			}
			vector<bool> ori_curModel=m_iter->second.second;

			//产生原三维结构待填充左上角坐标，判断位置是否能够填孔块(待填孔隙点位置全不为孔)
			//注意：控制坐标范围，不越界
			int fill_z=rand()%(m_biglayers-m_templesz+1);          //z坐标范围(0~b_layer-TempSize)
			int fill_y=rand()%(m_bigheight-m_templesz+1);           //y坐标范围(0~b_rows-TempSize)
			int fill_x=rand()%(m_bigwidth-m_templesz+1);           //x坐标范围(0~b_cols-TempSize)

			while(!Is_ROI_B(bPore_label,m_bigimg,ori_curModel,fill_z,fill_y,fill_x,m_templesz,m_biglayers,m_bigheight,m_bigwidth,flag01))   //选取位置不能填选取的小孔模块
			{
				fill_z=rand()%(m_biglayers-m_templesz+1);          
				fill_y=rand()%(m_bigheight-m_templesz+1);
				fill_x=rand()%(m_bigwidth-m_templesz+1);         
			}
			int a=2;
			int b=255;
			int new_label=rand()%(b-a+1)+a;     //为当前重建的小孔生成2~255的随机标签(0为图像背景，1为初始大孔标签)

			//填充所选的小模块到满足的位置，并同时改变当前三维结构的孔点数
			int m=-1;
			int new_poresize=0;     //统计新重建的小孔体积
			for (int z=fill_z;z<(fill_z+m_templesz);z++)
			{
				for (int y=fill_y;y<(fill_y+m_templesz);y++)
				{
					for (int x=fill_x;x<(fill_x+m_templesz);x++)
					{
						++m;
						//180312修正
						int t=0;
						if (ori_curModel[m]!=0)
						{
							t=255;
							m_bigimg[z][y][x]=t;
							chg_pNum++;     //整个三维结构总孔点数增加
							new_poresize++;
							bPore_label[z][y][x]=new_label;     //新增的小孔孔点标签
						}
					}
				}
			}

			while(b_up==true||b_down==true||b_left==true||b_right==true||b_front==true||b_back==true)      //某一方向仍能重建
			{
				tran++;

				/////////////////////////////////////判断整个重建过程是否停止/////////////////////////////////////
				if (IsReconstSuccess(savepath , chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					new_poreNum++; 
					//cout<<"已重建最后一个孔，为第"<<new_poreNum<<"个孔，该孔的体积为"<<new_poresize<<endl;
					return true;
				}
				
				if (b_up)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_up = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag1);
				}


				/////////////////////////////////////判断整个重建过程是否停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				
				if (b_down)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_down = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag2);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				
				if (b_left)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_left = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag3);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//右
				if (b_right)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_right = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag4);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//前
				if (b_front)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_front = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag5);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//后
				if (b_back)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_back = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag6);
				}

			}


			new_poreNum++; 
			//cout<<"已重建第"<<new_poreNum<<"个孔，该孔的体积为"<<new_poresize<<"，用时(min)"<<t<<endl;


			//////////////////////////////////不断更新&保存重建过程中的三维序列图////////////////////////////////////////
      /*
      QDir qdir(savepath);
      if (!qdir.exists("reconstruct")) qdir.mkdir("reconstruct");
      QImage img(m_bigwidth, m_bigheight, QImage::Format_Grayscale8);
      int i = 0, j = 0, k = 0, q = 0, white_point = 0;
      for (q = 0; q != m_biglayers; q++) {
        for (i = 0; i != m_bigheight; ++i) {
          uchar* lpix = img.scanLine(i);
          for (j = 0; j != m_bigwidth; ++j) {
            k = m_bigimg[q][i][j];
            *(lpix + j * 1) = k ;
          }
        }
        char name[100];
        sprintf(name, "%d.bmp", q);
        //最终图片名格式为数字编号，例“1.bmp”
        QString respath = savepath;
        respath.append("/reconstruct/").append(name);
        img.save(respath, nullptr, 100);
      }
      */

		}

    qDebug() << "chg_pNum: " << chg_pNum
                         << " tar_pNum: " << tar_pNum;
	}
  
  QDir qdir(savepath);
  if (!qdir.exists("reconstruct")) qdir.mkdir("reconstruct");
  QImage img(m_bigwidth, m_bigheight, QImage::Format_Grayscale8);
  int i = 0, j = 0, k = 0, q = 0, white_point = 0;
  for (q = 0; q != m_biglayers; q++) {
    for (i = 0; i != m_bigheight; ++i) {
      uchar* lpix = img.scanLine(i);
      for (j = 0; j != m_bigwidth; ++j) {
        k = m_bigimg[q][i][j];
        *(lpix + j * 1) = k ;
      }
    }
    char name[100];
    sprintf(name, "%d.bmp", q);
    //最终图片名格式为数字编号，例“1.bmp”
    QString respath = savepath;
    respath.append("/reconstruct/").append(name);
    img.save(respath, nullptr, 100);
  }
	//AfxMessageBox(_T("重建完成！"));
  ClearBigPorelabel();
  ClearBigImg();
  emit SetProcessVal(100);
	return true;
}


//建立小孔三维二值结构的孔模式集
//参数：
//用于保存多个小孔三维结构vector<Image3D> Image3Ds；记录建立的三维结构的个数int num；
//存储模式的集合pore_set<当前模式出现的概率double->模式块中孔点的个数area->！，该模式对应的外接正方体vector<vector<vector<int>>> >；(通过面积，将模式集划分成几个子模式集)
//17-10-16模式集map(pore_set)建立思路更新：将模块存为二进制式(背景0孔1)的vector<int>（eg.010110110）, 对应的key值即为相应的十进制数！
//17-10-16：添加参数map<double,int>probility，key值为模块（二进制计算值）,value值为同一模块出现次数！
//模式的尺寸
//10-19：用来保存建立的小孔三维图像
//11-27新增变量：multimap<int,double> 其中关键字key为当前模式中孔点的个数，对应value为模式十进制值。
bool PoreSet::Built3DImage01sPoreSet(const QString &filepath,int index_pos,int total_pos) {

  int seg = 30 / total_pos + 1;
  if (filepath.isEmpty()) return false;
  ClearSmallImg();
		//18-2-22：将之前几个map整合为一个multimap<pnum,pair<prob,model>>;
		//并找到将key由十进制转换为它之前对应的二进制串！
		//multimap<int,pair<string,pair<int,bitset<bs_bits>>>> m_pnum;   //由于在模式匹配的过程中，对于关键字的需求不高，关键字只是为了找模式出现的频率，所以最终模式集map中可以不存储关键字！   
		//map<string,int> str_prob;

    //保存小孔三维序列图
    QFileInfoList fileinfolist;
    QDir qdir(QFileInfo(filepath).absoluteDir());
    QStringList filters;
    filters << "*.png"
            << "*.jpg"
            << "*.bmp"
            << "*.jpeg";
    qdir.setNameFilters(filters);
    fileinfolist = qdir.entryInfoList();
    m_smalllayers = fileinfolist.size();

    m_smallimg = new int**[m_smalllayers];
    for (int i = 0; i < m_smalllayers; i++) {
      m_smallimg[i] = new int*[m_smallheight];
      for (int j = 0; j < m_smallheight; j++) {
        m_smallimg[i][j] = new int[m_smallwidth]();
      }
    }

    emit SetProcessVal(30 * index_pos / total_pos + seg / 3);

    QImage img;
    int index = 0;
    for (auto itr = fileinfolist.begin(); itr != fileinfolist.end(); itr++,index++) {
      if (m_shutdown) return false;
      img.load(itr->absoluteFilePath());
      QImage grayimg = img.convertToFormat(QImage::Format_Grayscale8);
      for (int i = 0; i < m_smallheight; i++) {
        const uchar* lpix = grayimg.scanLine(i);
        for (int j = 0; j < m_smallwidth; j++) {
          int pix = *(lpix + j * 1);
          m_smallimg[index][i][j] = (pix ? 255 : 0);
        }
      }
    }

    emit SetProcessVal(30 * index_pos / total_pos + seg*2 / 3);

		//存储小孔模式块！
		clock_t start2,finish2;
		start2=clock();

		//bitset<bs_bits> temp;     //180310：二进制式来存储模块
    vector<bool> temp(m_templesz*m_templesz*m_templesz, false);

		int n_33num=(m_templesz*m_templesz*m_templesz)/33;

		for (int z=0;z<=m_smalllayers-m_templesz;z++)        //保证模块不越界
		{
			for (int y=0;y<=m_smallheight-m_templesz;y++)
			{
				for (int x=0;x<=m_smallwidth-m_templesz;x++)
				{

          //取消标志
          if (m_shutdown) return false;

					int n=-1;       //vector下标，模块左上角点为第一个数(二进制最高位)，右下角点为最后一个数(二进制最低位)
					int p_num=0;   //模块中孔点个数
					string key_num; //180123：将模式用01表示，并直接转化为string作为关键字
					//模块内循环
					for (int k=0;k<m_templesz;k++)
					{
						for (int j=0;j<m_templesz;j++)
						{
							for (int i=0;i<m_templesz;i++)       //这里对temp是使用的下标访问，而不是push_back，所以必须先resize指定vector的大小
							{
								++n;      //这里n初始值为-1，在执行完i自增判断后再对n自增，避免n值多加，造成越界！
								if (m_smallimg[z+k][y+j][x+i]!=0)
								{
									temp[n]=1;    
								}
								else
								{
									temp[n]=0;
								}
							}
						}
					}
					for (int i1=0;i1<=n_33num;i1++)
					{
						long long ntrans=0;
						for (int j1=i1*33;j1<(i1+1)*33&&j1<(m_templesz*m_templesz*m_templesz);j1++)
						{
							int m=temp[j1];
							if (m!=0)
							{
								p_num++;
								int _2=j1-i1*33;
								ntrans+=pow(2.0,_2);
							}
						}
						key_num+=to_string(long long(ntrans));
					}

					//要是模块中孔点数不为0且不全是孔点，则保存该模块->(17-10-29修改：保存全是孔点的模块)
					if (p_num!=0)
					{
						pair<set<string>::iterator,bool> ret=m_strkey.insert(key_num);

						if (ret.second)  //若插入成功，即当前模块在模式集中第一次出现
						{
							//_mpnum.insert(make_pair(p_num,make_pair(1,temp)));
              m_Map.insert(make_pair(p_num,make_pair(1,temp)));
							//180125
							m_st_pnum.insert(p_num);        //11-27新增：存储模式对应的孔点数(不存重复值)
						}
						if (!ret.second)     //插入不成功，则将当前模块(也就是其对应二进制值key_num)在原有的个数上加1。
						{
							//180224
              multimap<int,pair<int,vector<bool>>>::iterator it,it_b,it_e;
							it_b=m_Map.equal_range(p_num).first;
							it_e=m_Map.equal_range(p_num).second;

							for (it=it_b;it!=it_e;it++)
							{
								if (it->second.second==temp)
								{
									it->second.first++;
								}
							}
							//注意，这里是保证了key值和value值是一一对应的，否则数据存储会出现损失！
						}

					}
					//0307：释放string占用的内存
					string().swap(key_num);
				}
			}
		}

    ClearSmallImg();  //释放Img3D
    emit SetProcessVal(30 * index_pos / total_pos + seg);
		//set<int>::iterator it;
		//it=m_st_pnum.end();
		//--it;
		//cout<<"当前模式集中模式孔点最大值为："<<*it<<endl;         //12-5：输出模式集中模式孔点最大值

		//cout<<"已存储第"<<num<<"个小孔三维结构的孔模式块"<<endl;
		int temp_num1=m_strkey.size();
		int temp_num2=m_Map.size();

		//if (temp_num1==temp_num2)
		//{
		//	cout<<"一共有"<<temp_num2<<"块，"<<"每块尺寸为"<<m_templesz;
		//}
		//else
		//{
		//	AfxMessageBox(_T("存储有误！"));
		//	return false;
		//}
    if (temp_num1 != temp_num2) return false;
		
 return true;

}

void PoreSet::ClearSmallImg() {
  if (m_smallimg) {
    for (int i = 0; i < m_smalllayers; i++) {
      for (int j = 0; j < m_smallheight; j++) {
        delete[] m_smallimg[i][j];
        m_smallimg[i][j] = nullptr;
      }
      delete[] m_smallimg[i];
    }
    delete[] m_smallimg;
    m_smallimg = nullptr;
  }
}

void PoreSet::ClearBigImg() {
  if (m_bigimg) {
    for (int i = 0; i < m_biglayers; i++) {
      for (int j = 0; j < m_bigheight; j++) {
        delete[] m_bigimg[i][j];
        m_bigimg[i][j] = nullptr;
      }
      delete[] m_bigimg[i];
    }
    delete[] m_bigimg;
    m_bigimg = nullptr;
  }
}

void PoreSet::ClearBigPorelabel() {
  if (bPore_label) {
    for (int i = 0; i < m_biglayers; i++) {
      for (int j = 0; j < m_bigheight; j++) {
        delete[] bPore_label[i][j];
        bPore_label[i][j] = nullptr;
      }
      delete[] bPore_label[i];
    }
    delete[] bPore_label;
    bPore_label = nullptr;
  }
}

//在大孔三维结构中，根据建立的小孔模式块集，来重建三维小孔结构
//参数：
//模式集——小孔模式块（小孔块存储二进制形式对应的十进制值<key>、存储为二进制式的小孔块<value>）
//记录每个小孔模式块出现的次数（小孔块存储二进制形式对应的十进制值<key>、当前小孔块在建立模式集过程中出现的次数<value>）
//模式块的尺寸
//11-27新增变量：multimap<int,double> 其中关键字key为当前模式中孔点的个数，对应value为模式十进制值。
//180222：只使用一个multimap来保存模式集，为multimap<模式孔点数，pair<模式概率，模式>>
/*
bool PoreSet::ReconstructSpores(const QString& filepath,const QString &savepath) { 

	/////////////////////////////////保存大孔三维结构/////////////////////////////////////////
  if (filepath.isEmpty()) return false;
  ClearBigImg();
  m_bigimg = new int**[m_biglayers];
  for (int i = 0; i < m_biglayers; i++) {
    m_bigimg[i] = new int*[m_bigheight];
    for (int j = 0; j < m_bigheight; j++) {
      m_bigimg[i][j] = new int[m_bigwidth]();
    }
  }

    QFileInfoList fileinfolist;
    QDir qdir(QFileInfo(filepath).absoluteDir());
    QStringList filters;
    filters << "*.png"
            << "*.jpg"
            << "*.bmp"
            << "*.jpeg";
    qdir.setNameFilters(filters);
    fileinfolist = qdir.entryInfoList();
    if (fileinfolist.size() != m_biglayers) return false;

    QImage img;
    int index = 0;
    for (auto itr = fileinfolist.begin(); itr != fileinfolist.end(); itr++,index++) {
      img.load(itr->absoluteFilePath());
      QImage grayimg = img.convertToFormat(QImage::Format_Grayscale8);
      for (int i = 0; i < m_bigheight; i++) {
        const uchar* lpix = grayimg.scanLine(i);
        for (int j = 0; j < m_bigwidth; j++) {
          int pix = *(lpix + j * 1);
          m_bigimg[index][i][j] = (pix ? 255 : 0);
        }
      }
    }

	double b_totalNum=m_biglayers*m_bigheight*m_bigwidth;   //三维大孔结构总像素点个数
	double b_porePnum=0;    //大孔三维结构原始孔点数



	///////////////////////////////////为大孔三维结构中的孔设定孔点标签///////////////////////////////////////

  //将初始大孔三维结构中的孔点设定标签，判断重建小孔是否与大孔连通
  ClearBigPorelabel();
  bPore_label = new int**[m_biglayers];  
  for (int i = 0; i < m_biglayers; i++) {
    m_bigimg[i] = new int*[m_bigheight];
    for (int j = 0; j < m_bigheight; j++) {
      m_bigimg[i][j] = new int[m_bigwidth]();
    }
  }
	for (int z=0;z<m_biglayers;z++)
	{
		for (int y=0;y<m_bigheight;y++)
		{
			for (int x=0;x<m_bigwidth;x++)
			{
				if (m_bigimg[z][y][x]!=0)   //孔点
				{
					bPore_label[z][y][x]=1;   //初始大孔三维结构中的孔点标签设为1
					b_porePnum++;
				}
				else
					bPore_label[z][y][x]=0;
			}
		}
	}
	//cout<<"大孔三维结构孔隙标签初始化完成！用时"<<time2<<"(min)"<<endl;


	//重建匹配思路：
	//根据填充的小孔模式块，作为该重建小孔的硬数据，再通过平移模板，获取新的待重建模式，在模式集中进行搜索匹配来重建该小孔剩余的孔点
	
	////////////////////////////////获取重建前和期望重建后的孔隙度(孔点个数)//////////////////////////////////////////
	//三维孔隙度(原始&期望值)
	double ori_p=(100*b_porePnum)/b_totalNum; 

	//CString cs_ori_p;
	//cs_ori_p.Format(_T("%.2f"),ori_p);
	//cout<<"大孔三维结构的初始孔隙度(百分数)为："<<cs_ori_p<<endl;

	//cout<<"请输入期望重建小孔后的三维结构孔隙度(百分数)(请保留两位小数)：";
	//double tar_p;
	//cin>>tar_p;

	double tar_p;

	double tar_pNum=b_totalNum*(tar_p/100.0);
	double chg_pNum=b_porePnum;        //统计重建过程中的孔点数(变化值)
	int deviation=10;      //允许的偏差值

	int dev=(tar_pNum-b_porePnum)*0.65;     //12-15：前期按照之前的孔重建停止条件重建的孔点数



	//////////////////////////////////开始重建////////////////////////////////////////
	int new_poreNum=0;        //统计重建孔的个数
	while(chg_pNum<(tar_pNum-deviation))    //判断是否继续重建小孔，当最终图像孔隙点数小于指定最小孔隙点数，可继续重建满足条件的小孔
	{   


		/////////////////////////////////孔重建及判断重建完成的思路/////////////////////////////////////////

		//向上下前后左右，平移模板，从而得到6个新模块(每次平移一个像素点)，为新模块从模式集中寻找最匹配的模块（相同相位点个数必须达到设定的阈值），将匹配的模块替换新模块
		//若在重建过程中，某一方向接触到已重建的孔，（在重建需要改变已有孔点相位的情况下）保持已重建孔点不变。当重建模板中已重建其他孔点占一半及以上时，停止在该方向的重建（两孔已连通）

		int tran=0;      //平移量

		//重建当前小孔:
		//重建的某方向与其他孔相连或平移后新模块全为0，该方向不再重建；当连续随机6个方向都不能重建时，停止该孔的重建！

		int a1,b1,c1;         //a、b、c分别为确定重建方向的z、y、x的系数
		srand((unsigned)time(NULL));
		int low=-1;
		int high=1;     //产生-1~1之间的随机数，随机确定重建方向

		bool b_up=true, b_down=true, b_front=true, b_back=true, b_left=true, b_right=true;
		int nflag1=-1,nflag2=-1,nflag3=-1,nflag4=-1,nflag5=-1,nflag6=-1;

		//12-15：当重建的孔隙度在前后孔隙度差值的10%之内时，按照之前的重建方法
		//       当在10%之外时，采用新的重建结束判断，只有当当前重建孔与其他重建孔连接时才结束，否则将平移分量变为0，重新从头开始重建
		int flag01;      

		if (chg_pNum<(b_porePnum+dev))           
		{
			flag01=1;          

			clock_t s,f;
			s=clock();

			//////////////////选取初始模式：首先随机确定子模式集，再在对应子模式集中随机选取初始模式块///////////////////////////////////
			srand((unsigned)time(NULL));
			int st_totalnum=m_st_pnum.size();
			//18-1-1修改：首先随机选取子模式集！（0~st_num-1）
			int r_st=rand()%(st_totalnum);         
			set<int>::const_iterator st_iter=m_st_pnum.begin();
			for (int i=0;i<r_st;i++)
			{
				++st_iter;
			}
			int c_modelpnum=*st_iter;      //当前选中的子模式集对应孔点数
			//18-1-1修改：再随机选取对应子模式集中模式块！（0~m-1）
			int m1=m_Map.count(c_modelpnum);
			int r_model=rand()%(m1);     
			multimap<int,pair<int,vector<bool> >>::const_iterator m_iter;
      

			m_iter=m_Map.equal_range(c_modelpnum).first;
			for (int i=0;i<r_model;i++)
			{
				++m_iter;
			}
			//bitset<bs_bits> ori_curModel=m_iter->second.second;
        vector<bool> ori_curModel = m_iter->second.second;

			//产生原三维结构待填充左上角坐标，判断位置是否能够填孔块(待填孔隙点位置全不为孔)
			//注意：控制坐标范围，不越界
			int fill_z=rand()%(m_biglayers-m_templesz+1);          //z坐标范围(0~b_layer-TempSize)
			int fill_y=rand()%(m_bigheight-m_templesz+1);           //y坐标范围(0~b_rows-TempSize)
			int fill_x=rand()%(m_bigwidth-m_templesz+1);           //x坐标范围(0~b_cols-TempSize)
      //选取位置不能填选取的小孔模块
      while (!Is_ROI_B(bPore_label, m_bigimg, ori_curModel,
                       fill_z, fill_y, fill_x, m_templesz,
                       m_biglayers, m_bigheight, m_bigwidth,
                       flag01)) {
        fill_z = rand() % (m_biglayers - m_templesz + 1);
        fill_y = rand() % (m_bigheight - m_templesz + 1);
        fill_x = rand() % (m_bigwidth - m_templesz + 1);
      }
			int a=2;
			int b=255;
			int new_label=rand()%(b-a+1)+a;     //为当前重建的小孔生成2~255的随机标签(0为图像背景，1为初始大孔标签)

			//填充所选的小模块到满足的位置，并同时改变当前三维结构的孔点数
			int m=-1;
			int new_poresize=0;     //统计新重建的小孔体积
			for (int z=fill_z;z<(fill_z+m_templesz);z++)
			{
				for (int y=fill_y;y<(fill_y+m_templesz);y++)
				{
					for (int x=fill_x;x<(fill_x+m_templesz);x++)
					{
						++m;
						//180312修正
						int t=0;
						if (ori_curModel[m]!=0)
						{
							t=255;
							m_bigimg[z][y][x]=t;
							chg_pNum++;     //整个三维结构总孔点数增加
							new_poresize++;
							bPore_label[z][y][x]=new_label;     //新增的小孔孔点标签
						}
					}
				}
			}

			while(b_up==true||b_down==true||b_left==true||b_right==true||b_front==true||b_back==true)      //某一方向仍能重建
			{
				tran++;

				/////////////////////////////////////判断整个重建过程是否停止/////////////////////////////////////
				if (IsReconstSuccess(savepath , chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					new_poreNum++; 
					//cout<<"已重建最后一个孔，为第"<<new_poreNum<<"个孔，该孔的体积为"<<new_poresize<<endl;
					return true;
				}
				
				if (b_up)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_up = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag1);
				}


				/////////////////////////////////////判断整个重建过程是否停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				
				if (b_down)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_down = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag2);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				
				if (b_left)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_left = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag3);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//右
				if (b_right)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_right = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag4);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//前
				if (b_front)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_front = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag5);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//后
				if (b_back)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_back = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag6);
				}

			}

			new_poreNum++; 
			f=clock();
			double t=(f-s)/CLOCKS_PER_SEC/60.0;

			//cout<<"已重建第"<<new_poreNum<<"个孔，该孔的体积为"<<new_poresize<<"，用时(min)"<<t<<endl;


			//////////////////////////////////不断更新&保存重建过程中的三维序列图////////////////////////////////////////
      
      QDir qdir(savepath);
      if (!qdir.exists("reconstruct")) qdir.mkdir("reconstruct");
      QImage img(m_bigwidth, m_bigheight, QImage::Format_Grayscale8);
      int i = 0, j = 0, k = 0, q = 0, white_point = 0;
      for (q = 0; q != m_biglayers; q++) {
        for (i = 0; i != m_bigheight; ++i) {
          uchar* lpix = img.scanLine(i);
          for (j = 0; j != m_bigwidth; ++j) {
            k = m_bigimg[q][i][j];
            *(lpix + j * 1) = k ;
          }
        }
        char name[100];
        sprintf(name, "%d.bmp", q);
        //最终图片名格式为数字编号，例“1.bmp”
        QString respath = savepath;
        respath.append("/reconstruct/").append(name);
        img.save(respath, nullptr, 100);
      }


		}
		else      //12-15：采用迭代重建的方法！只有当某一方向与其他孔连接才停止该孔的重建，否则将坐标偏移量置为0，从头开始重建
		{
			flag01=0;         //12-26：对于后百分之50的孔点，其放置位置必须可与任意孔靠近！ 

			clock_t s,f;
			s=clock();

			//18-1-1修改：初始孔块随机确定子模式集，再在对应子模式集中随机选取初始模式块
			srand((unsigned)time(NULL));
			int st_totalnum=m_st_pnum.size();
			//18-1-1修改：首先随机选取子模式集！（0~st_num-1）
			int r_st=rand()%(st_totalnum);         
			set<int>::const_iterator st_iter=m_st_pnum.begin();
			for (int i=0;i<r_st;i++)
			{
				++st_iter;
			}
			int c_modelpnum=*st_iter;      //当前选中的子模式集对应孔点数
			//18-1-1修改：再随机选取对应子模式集中模式块！（0~m-1）
			int m1=m_Map.count(c_modelpnum);
			int r_model=rand()%(m1);     
			multimap<int,pair<int,vector<bool>>>::const_iterator m_iter;
			m_iter=m_Map.equal_range(c_modelpnum).first;

			for (int i=0;i<r_model;i++)
			{
				++m_iter;
			}
			vector<bool> ori_curModel=m_iter->second.second;

			//产生原三维结构待填充左上角坐标，判断位置是否能够填孔块(待填孔隙点位置全不为孔)
			//注意：控制坐标范围，不越界
			int fill_z=rand()%(m_biglayers-m_templesz+1);          //z坐标范围(0~b_layer-TempSize)
			int fill_y=rand()%(m_bigheight-m_templesz+1);           //y坐标范围(0~b_rows-TempSize)
			int fill_x=rand()%(m_bigwidth-m_templesz+1);           //x坐标范围(0~b_cols-TempSize)

			while(!Is_ROI_B(bPore_label,m_bigimg,ori_curModel,fill_z,fill_y,fill_x,m_templesz,m_biglayers,m_bigheight,m_bigwidth,flag01))   //选取位置不能填选取的小孔模块
			{
				fill_z=rand()%(m_biglayers-m_templesz+1);          
				fill_y=rand()%(m_bigheight-m_templesz+1);
				fill_x=rand()%(m_bigwidth-m_templesz+1);         
			}
			int a=2;
			int b=255;
			int new_label=rand()%(b-a+1)+a;     //为当前重建的小孔生成2~255的随机标签(0为图像背景，1为初始大孔标签)

			//填充所选的小模块到满足的位置，并同时改变当前三维结构的孔点数
			int m=-1;
			int new_poresize=0;     //统计新重建的小孔体积
			for (int z=fill_z;z<(fill_z+m_templesz);z++)
			{
				for (int y=fill_y;y<(fill_y+m_templesz);y++)
				{
					for (int x=fill_x;x<(fill_x+m_templesz);x++)
					{
						++m;
						//180312修正
						int t=0;
						if (ori_curModel[m]!=0)
						{
							t=255;
							m_bigimg[z][y][x]=t;
							chg_pNum++;     //整个三维结构总孔点数增加
							new_poresize++;
							bPore_label[z][y][x]=new_label;     //新增的小孔孔点标签
						}
					}
				}
			}

			while(b_up==true||b_down==true||b_left==true||b_right==true||b_front==true||b_back==true)      //某一方向仍能重建
			{
				tran++;

				/////////////////////////////////////判断整个重建过程是否停止/////////////////////////////////////
				if (IsReconstSuccess(savepath , chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					new_poreNum++; 
					//cout<<"已重建最后一个孔，为第"<<new_poreNum<<"个孔，该孔的体积为"<<new_poresize<<endl;
					return true;
				}
				
				if (b_up)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_up = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag1);
				}


				/////////////////////////////////////判断整个重建过程是否停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				
				if (b_down)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_down = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag2);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				
				if (b_left)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_left = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag3);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//右
				if (b_right)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_right = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag4);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//前
				if (b_front)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_front = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag5);
				}


				/////////////////////////////////////判断整个重建过程停止/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//后
				if (b_back)
				{
					a1=rand()%(high-low+1)+low;
					b1=rand()%(high-low+1)+low;
					c1=rand()%(high-low+1)+low;
          b_back = _6direc_Reconst_Pore(
              fill_z, a1, fill_y, b1, fill_x, c1,
              tran, m_bigimg, bPore_label,
              new_label, chg_pNum, new_poresize,
              m_biglayers, m_bigheight,
              m_bigwidth, nflag6);
				}

			}


			new_poreNum++; 
			//cout<<"已重建第"<<new_poreNum<<"个孔，该孔的体积为"<<new_poresize<<"，用时(min)"<<t<<endl;


			//////////////////////////////////不断更新&保存重建过程中的三维序列图////////////////////////////////////////
      QDir qdir(savepath);
      if (!qdir.exists("reconstruct")) qdir.mkdir("reconstruct");
      QImage img(m_bigwidth, m_bigheight, QImage::Format_Grayscale8);
      int i = 0, j = 0, k = 0, q = 0, white_point = 0;
      for (q = 0; q != m_biglayers; q++) {
        for (i = 0; i != m_bigheight; ++i) {
          uchar* lpix = img.scanLine(i);
          for (j = 0; j != m_bigwidth; ++j) {
            k = m_bigimg[q][i][j];
            *(lpix + j * 1) = k ;
          }
        }
        char name[100];
        sprintf(name, "%d.bmp", q);
        //最终图片名格式为数字编号，例“1.bmp”
        QString respath = savepath;
        respath.append("/reconstruct/").append(name);
        img.save(respath, nullptr, 100);
      }

		}

	}
	//AfxMessageBox(_T("重建完成！"));
	return true;
}
*/

bool PoreSet::Is_ROI_B(int*** pore_labels, int*** Image3D, vector<bool>& ROI,
                       int z_layers, int y_rows, int x_cols, int ROI_size,
                       int _blayers, int _bRows, int _bCols, int flag01) {
  int e_point = 0;  //待填充区不满足条件的点个数
  int neighbor_porenum0 = 0;
  // 12-26：记录待填充位置附近其他孔的孔点数（判断当前位置是否靠近其他孔）
  int neighbor_porenum1 = 0;
  int n = -1;

  for (int z = z_layers; z < (z_layers + ROI_size); z++) {
    for (int y = y_rows; y < (y_rows + ROI_size); y++) {
      for (int x = x_cols; x < (x_cols + ROI_size); x++) {
        ++n;
        if (ROI[n] != 0 && Image3D[z][y][x] != 0)  //待填充孔点位置不为背景
        {
          e_point++;
          return false;
        } else  // 12-26：待填充孔点位置为背景
        {
          // 12-26：判断当前孔点的邻域（垂直6个方向的2个像素点长之内）附近是否有其他孔点
          //(12-31修改：将==写成了=)
          if (flag01 == 0 &&
              (((z - 1) >= 0 && pore_labels[z - 1][y][x] != 0) ||
               ((z + 1) < _blayers && pore_labels[z + 1][y][x] != 0) ||
               ((y - 1) >= 0 && pore_labels[z][y - 1][x] != 0) ||
               ((y + 1) < _bRows && pore_labels[z][y + 1][x] != 0) ||
               ((x - 1) >= 0 && pore_labels[z][y][x - 1] != 0) ||
               ((x + 1) < _bCols && pore_labels[z][y][x + 1] != 0))) {
            neighbor_porenum0++;
          } else if (flag01 == 1 &&
                     (((z - 1) >= 0 && pore_labels[z - 1][y][x] == 1) ||
                      ((z + 1) < _blayers && pore_labels[z + 1][y][x] == 1) ||
                      ((y - 1) >= 0 && pore_labels[z][y - 1][x] == 1) ||
                      ((y + 1) < _bRows && pore_labels[z][y + 1][x] == 1) ||
                      ((x - 1) >= 0 && pore_labels[z][y][x - 1] == 1) ||
                      ((x + 1) < _bCols && pore_labels[z][y][x + 1] == 1))) {
            neighbor_porenum1++;
          }
        }
      }
    }
  }
  //(12-31修改：将==写成了=)
  if ((flag01 == 0 && neighbor_porenum0 != 0) ||
      (flag01 == 1 && neighbor_porenum1 != 0)) {
    return true;
  } else {
    return false;
  }
}


//判断重建是否完成，完成则保存对应的图像
//参数：
//重建过程中的整体三维结构的孔点数、目标孔点数、孔点允许的偏差值、保存三维孔结构的容器、大孔三维结构的（层数、行数和列数）
bool PoreSet::IsReconstSuccess(const QString&savepath , double _chgnum, double trag_num, int dev,
                               int*** _3D, int _blayer, int _bRows,
                               int _bCols) {
  if (_chgnum < (trag_num + dev)) return false;
  if (savepath.isEmpty()) return false;

  /*
  QDir qdir(savepath);
  if (!qdir.exists("reconstruct")) qdir.mkdir("reconstruct");
  QImage img(_bCols, _bRows, QImage::Format_Grayscale8);
  int i = 0, j = 0, k = 0, q = 0, white_point = 0;
  for (q = 0; q != _blayer; q++) {
    for (i = 0; i != _bRows; ++i) {
      uchar* lpix = img.scanLine(i);
      for (j = 0; j != _bCols; ++j) {
        k = _3D[q][i][j];
        *(lpix + j * 1) = (k ? 255 : 0);
      }
    }
    char name[100];
    sprintf(name, "%d.bmp", q);
    //最终图片名格式为数字编号，例“1.bmp”
    QString respath = savepath;
    respath.append("/reconstruct/").append(name);
    img.save(respath, nullptr, 100);
  }
  */
  return true;
}

bool PoreSet::_6direc_Reconst_Pore(int fillz, int a, int filly, int b,
                                   int fillx, int c, int trans, int*** _3D,
                                   int*** _porelabel, int _newlabel,
                                   double& _chgnum, int& _newporesize,
                                   int _blayer, int _bRows, int _bCols,
                                   int& nflag) {
	int label_1_num=0;    //模块中包含的已重建其他孔标签的点数量

	int reconst_z=fillz+trans*a;    //平移后模块左上角点
	int reconst_y=filly+trans*b;
	int reconst_x=fillx+trans*c;

	//若平移后的坐标(10-28修改：模块的左上角坐标！！)越界！该方向上不能继续重建，返回false
  if (reconst_z < 0 || reconst_z > (_blayer - m_templesz) ||
      reconst_y < 0 || reconst_y > (_bRows - m_templesz) ||
      reconst_x < 0 || reconst_x > (_bCols - m_templesz)) {
    return false;
  }

	//180311
	//bitset<bs_bits> bs_newmodel;   //为了更快的进行比较
  vector<bool> bs_newmodel(m_templesz * m_templesz * m_templesz, false);

	string key;
	int n=-1;
	int new_modelpnum=0;    //平移后新模式的孔点数
	int other_pnum=0;      //平移后新模式中其他孔的孔点数

	for (int z=reconst_z;z<(reconst_z+m_templesz);z++)
	{
		for (int y=reconst_y;y<(reconst_y+m_templesz);y++)
		{
			for (int x=reconst_x;x<(reconst_x+m_templesz);x++)
			{
				++n;
				if (_porelabel[z][y][x]!=0/*==_newlabel*/)       //12-25：只把当前待重建的孔孔点存入新模式！不能将其他孔点也存入！
					                                             //18-1-2:把非背景点都考虑进来，只是最后重建设定不能改变大孔结构！小孔结构可不断改变
				{
					bs_newmodel[n]=1;
					if (_porelabel[z][y][x]==1)        //18-2-11：新模式中大孔结构的孔点的个数
					{
						other_pnum++;
					}
				}
				else
				{
					bs_newmodel[n]=0;
				}

			}
		}
	}
	int n_33num=(m_templesz*m_templesz*m_templesz)/33;
	for (int i2=0;i2<=n_33num;i2++)
	{
		long long ntrans=0;
		for (int j2=i2*33;j2<(i2+1)*33&&j2<(m_templesz*m_templesz*m_templesz);j2++)
		{
			int m=bs_newmodel[j2];
			if (m!=0)
			{
				new_modelpnum++;
				int _2=j2-i2*33;
				ntrans+=pow(2.0,_2);
			}
			key+=to_string(long long(ntrans));
		}
	}

	//若当前平移后的模块全为0，该方向上不能再继续匹配，返回false
	//18-2-11：或平移后模块全为大孔结构的孔点，也返回false
	if (new_modelpnum==0||other_pnum==m_templesz*m_templesz*m_templesz)  
	{
		nflag=0;      //12-15：模板中全为背景
		return false;
	}
	else
	{
		if (m_strkey.find(key)==m_strkey.end())   //待重建模块在模式集中！
		{
      //模式集寻找最相近的块
			int p_max=0;          //概率最大
			int min_diff=m_templesz*m_templesz*m_templesz+1;       //保存最小差异值！初始值设得非常大（主要是为了服务当待重建模式孔点数不在模式集中时，第一次找到最接近它的模式孔点数时不执行寻找）
			//bitset<bs_bits> best_model;     //存储寻找到的最佳模块
      vector<bool> best_model(
          m_templesz * m_templesz * m_templesz, false);

			int n=1;     //主要是将第一个满足条件的块的相应参数赋为min_diff等的初始值

			////////////////////////////////////11-27：寻找最匹配的模式//////////////////////////////////////

			//当前待搜索子模式的孔点数
			int cur_modelpnum;       

			stack<int> suit_pnum;        //将与待重建模式孔点数差异小于等于min_diff(当前找到的最小模式相位差异)的对应的孔点数存入

			set<int> already_deal;    //保存已遍历了对应子模式集的孔点数
			set<int>::iterator set_iter;

			cur_modelpnum=new_modelpnum;
			already_deal.insert(new_modelpnum);
			IsfindBestModel(bs_newmodel,cur_modelpnum,new_modelpnum,min_diff,p_max,n,suit_pnum,already_deal,best_model);

			while (!suit_pnum.empty())
			{
				cur_modelpnum=suit_pnum.top();
				suit_pnum.pop();     //将待处理孔点数弹出stack

				set_iter=already_deal.find(cur_modelpnum);

				if (set_iter==already_deal.end())          //当前模式孔点数对应子模式集未遍历！
				{
					int cur_pnum_diff=abs(cur_modelpnum-new_modelpnum);      //由于可能在模式集拥有子模式孔点数中，没有能够大于待重建孔点数的，所以该值可能为负
					if (cur_pnum_diff<0)
					{
						//AfxMessageBox(_T("abs函数取绝对值出错！"));
						return false;
					}

					if (cur_pnum_diff<=min_diff)      //由于min_diff可能会不断变小，用这个条件可以进一步缩小搜索的范围
					{
						already_deal.insert(cur_modelpnum);
						IsfindBestModel(bs_newmodel,cur_modelpnum,new_modelpnum,min_diff,p_max,n,suit_pnum,already_deal,best_model);
					}
				}
			}

			//将找到的最匹配模式替换待重建模式
      //12-4：对寻找到的模式是否存在做判定！--->12-5：若该条件满足，即在模式集中未找到孔点数大于当前模式的模式，该模式保留不重建
			if (best_model == vector<bool>(m_templesz*m_templesz*m_templesz,false))            
			{
				/*AfxMessageBox(_T("寻找到的最佳模式在模式集中并不存在！"));*/
				return true;
			}

			int k=-1;
			for (int z=reconst_z;z<(reconst_z+m_templesz);z++)
			{
				for (int y=reconst_y;y<(reconst_y+m_templesz);y++)
				{
					for (int x=reconst_x;x<(reconst_x+m_templesz);x++)
					{
						++k;
						int t=0;
						if (best_model[k]!=0)
						{
							t=255;
						}
						//180312修改的部分主要与这里有关！
						//但实际对重建结果好像无影响，只是增加了赋值的像素点个数
						//本来为孔点(1)的位置要再被赋值一遍为(255)，实际结果反而没问题。
						if (t!=_3D[z][y][x]         //12-4修改：由!=new_model[k]改为!=_3D[z][y][x]
						&&_porelabel[z][y][x]!=1)           //12-25修改：模式匹配不能更改初始的大孔孔点！大孔结构不能改变！

						{
							if (t!=0)        //12-26:匹配后，在当前点位置需由0->255（背景->孔）
							{
								_3D[z][y][x]=t;
								_porelabel[z][y][x]=_newlabel;
								_chgnum++;
								_newporesize++;
							}
							else            //12-26：匹配后，这里需要从255->0（孔->背景）
							{
								_3D[z][y][x]=t;        //18-1-2:非当前重建小孔点也能被改变，但不算入当前重建孔的体积（否则可能会得出孔尺寸为负的结果！）
								_chgnum--;

								if (_porelabel[z][y][x]==_newlabel)      //12-26：只将当前重建孔的孔点赋为背景0！而不改变其他孔点
								{
									_newporesize--;
								}
								_porelabel[z][y][x]=0;
							}
						}
					}
				}
			}
		}

		return true;       //当该孔不与其他孔连通时，会重建（模块不变或者变化）！
	}

}

bool PoreSet::IsfindBestModel(vector<bool>& bs_newmodel, int& cur_modelpnum,
                              int new_modelpnum, int& min_diff, int& p_max,
                              int& n, stack<int>& suit_pnum,
                              set<int> const& already_deal,
                              vector<bool>& best_m) {
	if (m_st_pnum.find(cur_modelpnum)!=m_st_pnum.end())         //在模式集中找到与当前模式孔点数相同的子模式集，定位到子模式集寻找最匹配模式
	{
		multimap<int,pair<int,vector<bool>>>::const_iterator mul_iter,b_iter,e_iter;
		
		b_iter=m_Map.equal_range(cur_modelpnum).first;
		e_iter=m_Map.equal_range(cur_modelpnum).second;

		for (mul_iter=b_iter;mul_iter!=e_iter;++mul_iter)
		{
			int cur_p=mul_iter->second.first;
			vector<bool> cur_model=mul_iter->second.second;
			int cur_diff=0;

			if (cur_model.size()!=m_templesz*m_templesz*m_templesz)
			{
				//AfxMessageBox(_T("请在代码中更改bitset的长度！"));
				return false;
			}

			//判断模式集中模式与新产生的模式相同位置像素点相位不同点的个数
      
      for (int i = 0; i < cur_model.size(); i++) {
        if (cur_model[i] ^ bs_newmodel[i]) cur_diff++;
      }
                        
			if (n==1)
			{
				p_max=cur_p;
				min_diff=cur_diff;
				best_m=cur_model;
				n=2;
			}

			if (cur_diff<min_diff)
			{
				min_diff=cur_diff;
				best_m=cur_model;
				p_max=cur_p;
			}
			else if (cur_diff==min_diff)
			{
				if (cur_p>p_max)
				{
					p_max=cur_p;
					best_m=cur_model;
				}
			}

		}

		//计算满足与待重建模式孔点数差异值小于等于当前找到的最小模式的相位差min_diff，判断寻找的最小差异能否在临近的孔点子模式集中找到差异更小或一样但概率更大的模式

		set<int>::const_iterator st_iter=m_st_pnum.begin();       
		for (;st_iter!=m_st_pnum.end();++st_iter)
		{
			int pnum=*st_iter;       //模式孔点数
			//180124：选取的最接近的模式，不一定使孔点数增加
			int p_diff=abs(pnum-new_modelpnum);           //与待重建孔点数之差

			//若孔点数与待重建孔块孔点数之差小于等于当前找到的最小相位差值min_diff且不等于0（即为new_modelpnum），将满足条件的孔点数存入stack
			if (p_diff<=min_diff)
			{
				set<int>::const_iterator ret;
				ret=already_deal.find(pnum);
				if (ret==already_deal.end())       //只将未遍历过的子模式集对应孔点数存入stack中！避免重复查找
				{
					suit_pnum.push(pnum);
				}
			}
		
		}
	}
	else          //未在模式集中找到与当前模式孔点cur_modelpnum（=new_modelpnum）一致的子模式集，只有在找new_modelpnum时才可能会有这种情况，因为剩下的suit_pnum中存的都是模式集
	{
		if (!m_st_pnum.empty())
		{
			set<int>::const_iterator st_iter;

			int pnum_diffmin=0;          //孔点数差异值最小值
			int best_pnum=0;             //满足孔点数差异最小的最佳模式孔点数(模式集中未保存孔点数为0的模式)
			int m=1;

			for (st_iter=m_st_pnum.begin();st_iter!=m_st_pnum.end();st_iter++)     //寻找模式集中与当前模式最接近的孔点数(若有两个孔点数与其一样接近，会选择比较大的那个！)
			{
				int diff=abs(*st_iter-cur_modelpnum);

				if (m==1)
				{
					pnum_diffmin=diff;
					best_pnum=*st_iter;
					m=2;
				}

				if (diff<=pnum_diffmin)
				{
					pnum_diffmin=diff;
					best_pnum=*st_iter;
				}
				else
				{
					break;          //由于set是按value值从小到大存储的，diff值应该呈现逐渐减小，在最接近的模式孔点数达到最小值后，再逐渐增加！
				}
		
			}

			suit_pnum.push(best_pnum);
			
		}

	}
	return true;
}
