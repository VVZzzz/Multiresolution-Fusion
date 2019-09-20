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

//������ά��׽ṹ
bool PoreSet::LoadBigPoreSet(const QString& filepath) { 
	/////////////////////////////////��������ά�ṹ/////////////////////////////////////////
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

	double b_totalNum=m_biglayers*m_bigheight*m_bigwidth;   //��ά��׽ṹ�����ص����
	b_porePnum=0;    //�����ά�ṹԭʼ�׵���


	///////////////////////////////////Ϊ�����ά�ṹ�еĿ��趨�׵��ǩ///////////////////////////////////////

  //����ʼ�����ά�ṹ�еĿ׵��趨��ǩ���ж��ؽ�С���Ƿ�������ͨ
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
				if (m_bigimg[z][y][x]!=0)   //�׵�
				{
					bPore_label[z][y][x]=1;   //��ʼ�����ά�ṹ�еĿ׵��ǩ��Ϊ1
					b_porePnum++;
				}
				else
					bPore_label[z][y][x]=0;
			}
		}
	}
	//cout<<"�����ά�ṹ��϶��ǩ��ʼ����ɣ���ʱ"<<time2<<"(min)"<<endl;
	//�ؽ�ƥ��˼·��
	//��������С��ģʽ�飬��Ϊ���ؽ�С�׵�Ӳ���ݣ���ͨ��ƽ��ģ�壬��ȡ�µĴ��ؽ�ģʽ����ģʽ���н�������ƥ�����ؽ���С��ʣ��Ŀ׵�
	////////////////////////////////��ȡ�ؽ�ǰ�������ؽ���Ŀ�϶��(�׵����)//////////////////////////////////////////
	//��ά��϶��(ԭʼ&����ֵ)
  /*
	double ori_p=(100*b_porePnum)/b_totalNum; 
	CString cs_ori_p;
	cs_ori_p.Format(_T("%.2f"),ori_p);
	cout<<"�����ά�ṹ�ĳ�ʼ��϶��(�ٷ���)Ϊ��"<<cs_ori_p<<endl;

	cout<<"�����������ؽ�С�׺����ά�ṹ��϶��(�ٷ���)(�뱣����λС��)��";
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
	double chg_pNum=b_porePnum;        //ͳ���ؽ������еĿ׵���(�仯ֵ)
	int deviation=10;      //�����ƫ��ֵ

	int dev=(tar_pNum-b_porePnum)*0.65;     //12-15��ǰ�ڰ���֮ǰ�Ŀ��ؽ�ֹͣ�����ؽ��Ŀ׵���

	//////////////////////////////////��ʼ�ؽ�////////////////////////////////////////
	int new_poreNum=0;        //ͳ���ؽ��׵ĸ���
	while(chg_pNum<(tar_pNum-deviation))    //�ж��Ƿ�����ؽ�С�ף�������ͼ���϶����С��ָ����С��϶�������ɼ����ؽ�����������С��
	{   
    emit SetProcessVal(30 + 69.0 * chg_pNum / (tar_pNum - deviation));
    if (m_shutdown) return false;


		/////////////////////////////////���ؽ����ж��ؽ���ɵ�˼·/////////////////////////////////////////

		//������ǰ�����ң�ƽ��ģ�壬�Ӷ��õ�6����ģ��(ÿ��ƽ��һ�����ص�)��Ϊ��ģ���ģʽ����Ѱ����ƥ���ģ�飨��ͬ��λ���������ﵽ�趨����ֵ������ƥ���ģ���滻��ģ��
		//�����ؽ������У�ĳһ����Ӵ������ؽ��Ŀף������ؽ���Ҫ�ı����п׵���λ������£��������ؽ��׵㲻�䡣���ؽ�ģ�������ؽ������׵�ռһ�뼰����ʱ��ֹͣ�ڸ÷�����ؽ�����������ͨ��

		int tran=0;      //ƽ����

		//�ؽ���ǰС��:
		//�ؽ���ĳ������������������ƽ�ƺ���ģ��ȫΪ0���÷������ؽ������������6�����򶼲����ؽ�ʱ��ֹͣ�ÿ׵��ؽ���

		int a1,b1,c1;         //a��b��c�ֱ�Ϊȷ���ؽ������z��y��x��ϵ��
		srand((unsigned)time(NULL));
		int low=-1;
		int high=1;     //����-1~1֮�������������ȷ���ؽ�����

		bool b_up=true, b_down=true, b_front=true, b_back=true, b_left=true, b_right=true;
		int nflag1=-1,nflag2=-1,nflag3=-1,nflag4=-1,nflag5=-1,nflag6=-1;

		//12-15�����ؽ��Ŀ�϶����ǰ���϶�Ȳ�ֵ��10%֮��ʱ������֮ǰ���ؽ�����
		//       ����10%֮��ʱ�������µ��ؽ������жϣ�ֻ�е���ǰ�ؽ����������ؽ�������ʱ�Ž���������ƽ�Ʒ�����Ϊ0�����´�ͷ��ʼ�ؽ�
		int flag01;      

		if (chg_pNum<(b_porePnum+dev))           
		{
			flag01=1;          

			//////////////////ѡȡ��ʼģʽ���������ȷ����ģʽ�������ڶ�Ӧ��ģʽ�������ѡȡ��ʼģʽ��///////////////////////////////////
			srand((unsigned)time(NULL));
			int st_totalnum=m_st_pnum.size();
			//18-1-1�޸ģ��������ѡȡ��ģʽ������0~st_num-1��
			int r_st=rand()%(st_totalnum);         
			set<int>::const_iterator st_iter=m_st_pnum.begin();
			for (int i=0;i<r_st;i++)
			{
				++st_iter;
			}
			int c_modelpnum=*st_iter;      //��ǰѡ�е���ģʽ����Ӧ�׵���
			//18-1-1�޸ģ������ѡȡ��Ӧ��ģʽ����ģʽ�飡��0~m-1��
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

			//����ԭ��ά�ṹ��������Ͻ����꣬�ж�λ���Ƿ��ܹ���׿�(�����϶��λ��ȫ��Ϊ��)
			//ע�⣺�������귶Χ����Խ��
			int fill_z=rand()%(m_biglayers-m_templesz+1);          //z���귶Χ(0~b_layer-TempSize)
			int fill_y=rand()%(m_bigheight-m_templesz+1);           //y���귶Χ(0~b_rows-TempSize)
			int fill_x=rand()%(m_bigwidth-m_templesz+1);           //x���귶Χ(0~b_cols-TempSize)
      //ѡȡλ�ò�����ѡȡ��С��ģ��
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
			int new_label=rand()%(b-a+1)+a;     //Ϊ��ǰ�ؽ���С������2~255�������ǩ(0Ϊͼ�񱳾���1Ϊ��ʼ��ױ�ǩ)

			//�����ѡ��Сģ�鵽�����λ�ã���ͬʱ�ı䵱ǰ��ά�ṹ�Ŀ׵���
			int m=-1;
			int new_poresize=0;     //ͳ�����ؽ���С�����
			for (int z=fill_z;z<(fill_z+m_templesz);z++)
			{
				for (int y=fill_y;y<(fill_y+m_templesz);y++)
				{
					for (int x=fill_x;x<(fill_x+m_templesz);x++)
					{
						++m;
						//180312����
						int t=0;
						//if (ori_curModel[m]!=0)
						if (ori_curModel[m])
						{
							t=255;
							m_bigimg[z][y][x]=t;
							chg_pNum++;     //������ά�ṹ�ܿ׵�������
							new_poresize++;
							bPore_label[z][y][x]=new_label;     //������С�׿׵��ǩ
						}
					}
				}
			}

			while(b_up==true||b_down==true||b_left==true||b_right==true||b_front==true||b_back==true)      //ĳһ���������ؽ�
			{
				tran++;

				/////////////////////////////////////�ж������ؽ������Ƿ�ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath , chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					new_poreNum++; 
					//cout<<"���ؽ����һ���ף�Ϊ��"<<new_poreNum<<"���ף��ÿ׵����Ϊ"<<new_poresize<<endl;
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


				/////////////////////////////////////�ж������ؽ������Ƿ�ֹͣ/////////////////////////////////////
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//��
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//ǰ
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//��
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

			//cout<<"���ؽ���"<<new_poreNum<<"���ף��ÿ׵����Ϊ"<<new_poresize<<"����ʱ(min)"<<t<<endl;


			//////////////////////////////////���ϸ���&�����ؽ������е���ά����ͼ////////////////////////////////////////
      //qDebug() << "������ά�ṹ�ļ���";
      
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
        //����ͼƬ����ʽΪ���ֱ�ţ�����1.bmp��
        QString respath = savepath;
        respath.append("/reconstruct/").append(name);
        img.save(respath, nullptr, 100);
      }
      */
		}
		else      //12-15�����õ����ؽ��ķ�����ֻ�е�ĳһ���������������Ӳ�ֹͣ�ÿ׵��ؽ�����������ƫ������Ϊ0����ͷ��ʼ�ؽ�
		{
			flag01=0;         //12-26�����ں�ٷ�֮50�Ŀ׵㣬�����λ�ñ����������׿����� 

			//18-1-1�޸ģ���ʼ�׿����ȷ����ģʽ�������ڶ�Ӧ��ģʽ�������ѡȡ��ʼģʽ��
			srand((unsigned)time(NULL));
			int st_totalnum=m_st_pnum.size();
			//18-1-1�޸ģ��������ѡȡ��ģʽ������0~st_num-1��
			int r_st=rand()%(st_totalnum);         
			set<int>::const_iterator st_iter=m_st_pnum.begin();
			for (int i=0;i<r_st;i++)
			{
				++st_iter;
			}
			int c_modelpnum=*st_iter;      //��ǰѡ�е���ģʽ����Ӧ�׵���
			//18-1-1�޸ģ������ѡȡ��Ӧ��ģʽ����ģʽ�飡��0~m-1��
			int m1=m_Map.count(c_modelpnum);
			int r_model=rand()%(m1);     
			multimap<int,pair<int,vector<bool>>>::const_iterator m_iter;
			m_iter=m_Map.equal_range(c_modelpnum).first;

			for (int i=0;i<r_model;i++)
			{
				++m_iter;
			}
			vector<bool> ori_curModel=m_iter->second.second;

			//����ԭ��ά�ṹ��������Ͻ����꣬�ж�λ���Ƿ��ܹ���׿�(�����϶��λ��ȫ��Ϊ��)
			//ע�⣺�������귶Χ����Խ��
			int fill_z=rand()%(m_biglayers-m_templesz+1);          //z���귶Χ(0~b_layer-TempSize)
			int fill_y=rand()%(m_bigheight-m_templesz+1);           //y���귶Χ(0~b_rows-TempSize)
			int fill_x=rand()%(m_bigwidth-m_templesz+1);           //x���귶Χ(0~b_cols-TempSize)

			while(!Is_ROI_B(bPore_label,m_bigimg,ori_curModel,fill_z,fill_y,fill_x,m_templesz,m_biglayers,m_bigheight,m_bigwidth,flag01))   //ѡȡλ�ò�����ѡȡ��С��ģ��
			{
				fill_z=rand()%(m_biglayers-m_templesz+1);          
				fill_y=rand()%(m_bigheight-m_templesz+1);
				fill_x=rand()%(m_bigwidth-m_templesz+1);         
			}
			int a=2;
			int b=255;
			int new_label=rand()%(b-a+1)+a;     //Ϊ��ǰ�ؽ���С������2~255�������ǩ(0Ϊͼ�񱳾���1Ϊ��ʼ��ױ�ǩ)

			//�����ѡ��Сģ�鵽�����λ�ã���ͬʱ�ı䵱ǰ��ά�ṹ�Ŀ׵���
			int m=-1;
			int new_poresize=0;     //ͳ�����ؽ���С�����
			for (int z=fill_z;z<(fill_z+m_templesz);z++)
			{
				for (int y=fill_y;y<(fill_y+m_templesz);y++)
				{
					for (int x=fill_x;x<(fill_x+m_templesz);x++)
					{
						++m;
						//180312����
						int t=0;
						if (ori_curModel[m]!=0)
						{
							t=255;
							m_bigimg[z][y][x]=t;
							chg_pNum++;     //������ά�ṹ�ܿ׵�������
							new_poresize++;
							bPore_label[z][y][x]=new_label;     //������С�׿׵��ǩ
						}
					}
				}
			}

			while(b_up==true||b_down==true||b_left==true||b_right==true||b_front==true||b_back==true)      //ĳһ���������ؽ�
			{
				tran++;

				/////////////////////////////////////�ж������ؽ������Ƿ�ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath , chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					new_poreNum++; 
					//cout<<"���ؽ����һ���ף�Ϊ��"<<new_poreNum<<"���ף��ÿ׵����Ϊ"<<new_poresize<<endl;
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


				/////////////////////////////////////�ж������ؽ������Ƿ�ֹͣ/////////////////////////////////////
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//��
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//ǰ
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//��
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
			//cout<<"���ؽ���"<<new_poreNum<<"���ף��ÿ׵����Ϊ"<<new_poresize<<"����ʱ(min)"<<t<<endl;


			//////////////////////////////////���ϸ���&�����ؽ������е���ά����ͼ////////////////////////////////////////
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
        //����ͼƬ����ʽΪ���ֱ�ţ�����1.bmp��
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
    //����ͼƬ����ʽΪ���ֱ�ţ�����1.bmp��
    QString respath = savepath;
    respath.append("/reconstruct/").append(name);
    img.save(respath, nullptr, 100);
  }
	//AfxMessageBox(_T("�ؽ���ɣ�"));
  ClearBigPorelabel();
  ClearBigImg();
  emit SetProcessVal(100);
	return true;
}


//����С����ά��ֵ�ṹ�Ŀ�ģʽ��
//������
//���ڱ�����С����ά�ṹvector<Image3D> Image3Ds����¼��������ά�ṹ�ĸ���int num��
//�洢ģʽ�ļ���pore_set<��ǰģʽ���ֵĸ���double->ģʽ���п׵�ĸ���area->������ģʽ��Ӧ�����������vector<vector<vector<int>>> >��(ͨ���������ģʽ�����ֳɼ�����ģʽ��)
//17-10-16ģʽ��map(pore_set)����˼·���£���ģ���Ϊ������ʽ(����0��1)��vector<int>��eg.010110110��, ��Ӧ��keyֵ��Ϊ��Ӧ��ʮ��������
//17-10-16����Ӳ���map<double,int>probility��keyֵΪģ�飨�����Ƽ���ֵ��,valueֵΪͬһģ����ִ�����
//ģʽ�ĳߴ�
//10-19���������潨����С����άͼ��
//11-27����������multimap<int,double> ���йؼ���keyΪ��ǰģʽ�п׵�ĸ�������ӦvalueΪģʽʮ����ֵ��
bool PoreSet::Built3DImage01sPoreSet(const QString &filepath,int index_pos,int total_pos) {

  int seg = 30 / total_pos + 1;
  if (filepath.isEmpty()) return false;
  ClearSmallImg();
		//18-2-22����֮ǰ����map����Ϊһ��multimap<pnum,pair<prob,model>>;
		//���ҵ���key��ʮ����ת��Ϊ��֮ǰ��Ӧ�Ķ����ƴ���
		//multimap<int,pair<string,pair<int,bitset<bs_bits>>>> m_pnum;   //������ģʽƥ��Ĺ����У����ڹؼ��ֵ����󲻸ߣ��ؼ���ֻ��Ϊ����ģʽ���ֵ�Ƶ�ʣ���������ģʽ��map�п��Բ��洢�ؼ��֣�   
		//map<string,int> str_prob;

    //����С����ά����ͼ
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

		//�洢С��ģʽ�飡
		clock_t start2,finish2;
		start2=clock();

		//bitset<bs_bits> temp;     //180310��������ʽ���洢ģ��
    vector<bool> temp(m_templesz*m_templesz*m_templesz, false);

		int n_33num=(m_templesz*m_templesz*m_templesz)/33;

		for (int z=0;z<=m_smalllayers-m_templesz;z++)        //��֤ģ�鲻Խ��
		{
			for (int y=0;y<=m_smallheight-m_templesz;y++)
			{
				for (int x=0;x<=m_smallwidth-m_templesz;x++)
				{

          //ȡ����־
          if (m_shutdown) return false;

					int n=-1;       //vector�±꣬ģ�����Ͻǵ�Ϊ��һ����(���������λ)�����½ǵ�Ϊ���һ����(���������λ)
					int p_num=0;   //ģ���п׵����
					string key_num; //180123����ģʽ��01��ʾ����ֱ��ת��Ϊstring��Ϊ�ؼ���
					//ģ����ѭ��
					for (int k=0;k<m_templesz;k++)
					{
						for (int j=0;j<m_templesz;j++)
						{
							for (int i=0;i<m_templesz;i++)       //�����temp��ʹ�õ��±���ʣ�������push_back�����Ա�����resizeָ��vector�Ĵ�С
							{
								++n;      //����n��ʼֵΪ-1����ִ����i�����жϺ��ٶ�n����������nֵ��ӣ����Խ�磡
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

					//Ҫ��ģ���п׵�����Ϊ0�Ҳ�ȫ�ǿ׵㣬�򱣴��ģ��->(17-10-29�޸ģ�����ȫ�ǿ׵��ģ��)
					if (p_num!=0)
					{
						pair<set<string>::iterator,bool> ret=m_strkey.insert(key_num);

						if (ret.second)  //������ɹ�������ǰģ����ģʽ���е�һ�γ���
						{
							//_mpnum.insert(make_pair(p_num,make_pair(1,temp)));
              m_Map.insert(make_pair(p_num,make_pair(1,temp)));
							//180125
							m_st_pnum.insert(p_num);        //11-27�������洢ģʽ��Ӧ�Ŀ׵���(�����ظ�ֵ)
						}
						if (!ret.second)     //���벻�ɹ����򽫵�ǰģ��(Ҳ�������Ӧ������ֵkey_num)��ԭ�еĸ����ϼ�1��
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
							//ע�⣬�����Ǳ�֤��keyֵ��valueֵ��һһ��Ӧ�ģ��������ݴ洢�������ʧ��
						}

					}
					//0307���ͷ�stringռ�õ��ڴ�
					string().swap(key_num);
				}
			}
		}

    ClearSmallImg();  //�ͷ�Img3D
    emit SetProcessVal(30 * index_pos / total_pos + seg);
		//set<int>::iterator it;
		//it=m_st_pnum.end();
		//--it;
		//cout<<"��ǰģʽ����ģʽ�׵����ֵΪ��"<<*it<<endl;         //12-5�����ģʽ����ģʽ�׵����ֵ

		//cout<<"�Ѵ洢��"<<num<<"��С����ά�ṹ�Ŀ�ģʽ��"<<endl;
		int temp_num1=m_strkey.size();
		int temp_num2=m_Map.size();

		//if (temp_num1==temp_num2)
		//{
		//	cout<<"һ����"<<temp_num2<<"�飬"<<"ÿ��ߴ�Ϊ"<<m_templesz;
		//}
		//else
		//{
		//	AfxMessageBox(_T("�洢����"));
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

//�ڴ����ά�ṹ�У����ݽ�����С��ģʽ�鼯�����ؽ���άС�׽ṹ
//������
//ģʽ������С��ģʽ�飨С�׿�洢��������ʽ��Ӧ��ʮ����ֵ<key>���洢Ϊ������ʽ��С�׿�<value>��
//��¼ÿ��С��ģʽ����ֵĴ�����С�׿�洢��������ʽ��Ӧ��ʮ����ֵ<key>����ǰС�׿��ڽ���ģʽ�������г��ֵĴ���<value>��
//ģʽ��ĳߴ�
//11-27����������multimap<int,double> ���йؼ���keyΪ��ǰģʽ�п׵�ĸ�������ӦvalueΪģʽʮ����ֵ��
//180222��ֻʹ��һ��multimap������ģʽ����Ϊmultimap<ģʽ�׵�����pair<ģʽ���ʣ�ģʽ>>
/*
bool PoreSet::ReconstructSpores(const QString& filepath,const QString &savepath) { 

	/////////////////////////////////��������ά�ṹ/////////////////////////////////////////
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

	double b_totalNum=m_biglayers*m_bigheight*m_bigwidth;   //��ά��׽ṹ�����ص����
	double b_porePnum=0;    //�����ά�ṹԭʼ�׵���



	///////////////////////////////////Ϊ�����ά�ṹ�еĿ��趨�׵��ǩ///////////////////////////////////////

  //����ʼ�����ά�ṹ�еĿ׵��趨��ǩ���ж��ؽ�С���Ƿ�������ͨ
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
				if (m_bigimg[z][y][x]!=0)   //�׵�
				{
					bPore_label[z][y][x]=1;   //��ʼ�����ά�ṹ�еĿ׵��ǩ��Ϊ1
					b_porePnum++;
				}
				else
					bPore_label[z][y][x]=0;
			}
		}
	}
	//cout<<"�����ά�ṹ��϶��ǩ��ʼ����ɣ���ʱ"<<time2<<"(min)"<<endl;


	//�ؽ�ƥ��˼·��
	//��������С��ģʽ�飬��Ϊ���ؽ�С�׵�Ӳ���ݣ���ͨ��ƽ��ģ�壬��ȡ�µĴ��ؽ�ģʽ����ģʽ���н�������ƥ�����ؽ���С��ʣ��Ŀ׵�
	
	////////////////////////////////��ȡ�ؽ�ǰ�������ؽ���Ŀ�϶��(�׵����)//////////////////////////////////////////
	//��ά��϶��(ԭʼ&����ֵ)
	double ori_p=(100*b_porePnum)/b_totalNum; 

	//CString cs_ori_p;
	//cs_ori_p.Format(_T("%.2f"),ori_p);
	//cout<<"�����ά�ṹ�ĳ�ʼ��϶��(�ٷ���)Ϊ��"<<cs_ori_p<<endl;

	//cout<<"�����������ؽ�С�׺����ά�ṹ��϶��(�ٷ���)(�뱣����λС��)��";
	//double tar_p;
	//cin>>tar_p;

	double tar_p;

	double tar_pNum=b_totalNum*(tar_p/100.0);
	double chg_pNum=b_porePnum;        //ͳ���ؽ������еĿ׵���(�仯ֵ)
	int deviation=10;      //�����ƫ��ֵ

	int dev=(tar_pNum-b_porePnum)*0.65;     //12-15��ǰ�ڰ���֮ǰ�Ŀ��ؽ�ֹͣ�����ؽ��Ŀ׵���



	//////////////////////////////////��ʼ�ؽ�////////////////////////////////////////
	int new_poreNum=0;        //ͳ���ؽ��׵ĸ���
	while(chg_pNum<(tar_pNum-deviation))    //�ж��Ƿ�����ؽ�С�ף�������ͼ���϶����С��ָ����С��϶�������ɼ����ؽ�����������С��
	{   


		/////////////////////////////////���ؽ����ж��ؽ���ɵ�˼·/////////////////////////////////////////

		//������ǰ�����ң�ƽ��ģ�壬�Ӷ��õ�6����ģ��(ÿ��ƽ��һ�����ص�)��Ϊ��ģ���ģʽ����Ѱ����ƥ���ģ�飨��ͬ��λ���������ﵽ�趨����ֵ������ƥ���ģ���滻��ģ��
		//�����ؽ������У�ĳһ����Ӵ������ؽ��Ŀף������ؽ���Ҫ�ı����п׵���λ������£��������ؽ��׵㲻�䡣���ؽ�ģ�������ؽ������׵�ռһ�뼰����ʱ��ֹͣ�ڸ÷�����ؽ�����������ͨ��

		int tran=0;      //ƽ����

		//�ؽ���ǰС��:
		//�ؽ���ĳ������������������ƽ�ƺ���ģ��ȫΪ0���÷������ؽ������������6�����򶼲����ؽ�ʱ��ֹͣ�ÿ׵��ؽ���

		int a1,b1,c1;         //a��b��c�ֱ�Ϊȷ���ؽ������z��y��x��ϵ��
		srand((unsigned)time(NULL));
		int low=-1;
		int high=1;     //����-1~1֮�������������ȷ���ؽ�����

		bool b_up=true, b_down=true, b_front=true, b_back=true, b_left=true, b_right=true;
		int nflag1=-1,nflag2=-1,nflag3=-1,nflag4=-1,nflag5=-1,nflag6=-1;

		//12-15�����ؽ��Ŀ�϶����ǰ���϶�Ȳ�ֵ��10%֮��ʱ������֮ǰ���ؽ�����
		//       ����10%֮��ʱ�������µ��ؽ������жϣ�ֻ�е���ǰ�ؽ����������ؽ�������ʱ�Ž���������ƽ�Ʒ�����Ϊ0�����´�ͷ��ʼ�ؽ�
		int flag01;      

		if (chg_pNum<(b_porePnum+dev))           
		{
			flag01=1;          

			clock_t s,f;
			s=clock();

			//////////////////ѡȡ��ʼģʽ���������ȷ����ģʽ�������ڶ�Ӧ��ģʽ�������ѡȡ��ʼģʽ��///////////////////////////////////
			srand((unsigned)time(NULL));
			int st_totalnum=m_st_pnum.size();
			//18-1-1�޸ģ��������ѡȡ��ģʽ������0~st_num-1��
			int r_st=rand()%(st_totalnum);         
			set<int>::const_iterator st_iter=m_st_pnum.begin();
			for (int i=0;i<r_st;i++)
			{
				++st_iter;
			}
			int c_modelpnum=*st_iter;      //��ǰѡ�е���ģʽ����Ӧ�׵���
			//18-1-1�޸ģ������ѡȡ��Ӧ��ģʽ����ģʽ�飡��0~m-1��
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

			//����ԭ��ά�ṹ��������Ͻ����꣬�ж�λ���Ƿ��ܹ���׿�(�����϶��λ��ȫ��Ϊ��)
			//ע�⣺�������귶Χ����Խ��
			int fill_z=rand()%(m_biglayers-m_templesz+1);          //z���귶Χ(0~b_layer-TempSize)
			int fill_y=rand()%(m_bigheight-m_templesz+1);           //y���귶Χ(0~b_rows-TempSize)
			int fill_x=rand()%(m_bigwidth-m_templesz+1);           //x���귶Χ(0~b_cols-TempSize)
      //ѡȡλ�ò�����ѡȡ��С��ģ��
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
			int new_label=rand()%(b-a+1)+a;     //Ϊ��ǰ�ؽ���С������2~255�������ǩ(0Ϊͼ�񱳾���1Ϊ��ʼ��ױ�ǩ)

			//�����ѡ��Сģ�鵽�����λ�ã���ͬʱ�ı䵱ǰ��ά�ṹ�Ŀ׵���
			int m=-1;
			int new_poresize=0;     //ͳ�����ؽ���С�����
			for (int z=fill_z;z<(fill_z+m_templesz);z++)
			{
				for (int y=fill_y;y<(fill_y+m_templesz);y++)
				{
					for (int x=fill_x;x<(fill_x+m_templesz);x++)
					{
						++m;
						//180312����
						int t=0;
						if (ori_curModel[m]!=0)
						{
							t=255;
							m_bigimg[z][y][x]=t;
							chg_pNum++;     //������ά�ṹ�ܿ׵�������
							new_poresize++;
							bPore_label[z][y][x]=new_label;     //������С�׿׵��ǩ
						}
					}
				}
			}

			while(b_up==true||b_down==true||b_left==true||b_right==true||b_front==true||b_back==true)      //ĳһ���������ؽ�
			{
				tran++;

				/////////////////////////////////////�ж������ؽ������Ƿ�ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath , chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					new_poreNum++; 
					//cout<<"���ؽ����һ���ף�Ϊ��"<<new_poreNum<<"���ף��ÿ׵����Ϊ"<<new_poresize<<endl;
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


				/////////////////////////////////////�ж������ؽ������Ƿ�ֹͣ/////////////////////////////////////
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//��
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//ǰ
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//��
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

			//cout<<"���ؽ���"<<new_poreNum<<"���ף��ÿ׵����Ϊ"<<new_poresize<<"����ʱ(min)"<<t<<endl;


			//////////////////////////////////���ϸ���&�����ؽ������е���ά����ͼ////////////////////////////////////////
      
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
        //����ͼƬ����ʽΪ���ֱ�ţ�����1.bmp��
        QString respath = savepath;
        respath.append("/reconstruct/").append(name);
        img.save(respath, nullptr, 100);
      }


		}
		else      //12-15�����õ����ؽ��ķ�����ֻ�е�ĳһ���������������Ӳ�ֹͣ�ÿ׵��ؽ�����������ƫ������Ϊ0����ͷ��ʼ�ؽ�
		{
			flag01=0;         //12-26�����ں�ٷ�֮50�Ŀ׵㣬�����λ�ñ����������׿����� 

			clock_t s,f;
			s=clock();

			//18-1-1�޸ģ���ʼ�׿����ȷ����ģʽ�������ڶ�Ӧ��ģʽ�������ѡȡ��ʼģʽ��
			srand((unsigned)time(NULL));
			int st_totalnum=m_st_pnum.size();
			//18-1-1�޸ģ��������ѡȡ��ģʽ������0~st_num-1��
			int r_st=rand()%(st_totalnum);         
			set<int>::const_iterator st_iter=m_st_pnum.begin();
			for (int i=0;i<r_st;i++)
			{
				++st_iter;
			}
			int c_modelpnum=*st_iter;      //��ǰѡ�е���ģʽ����Ӧ�׵���
			//18-1-1�޸ģ������ѡȡ��Ӧ��ģʽ����ģʽ�飡��0~m-1��
			int m1=m_Map.count(c_modelpnum);
			int r_model=rand()%(m1);     
			multimap<int,pair<int,vector<bool>>>::const_iterator m_iter;
			m_iter=m_Map.equal_range(c_modelpnum).first;

			for (int i=0;i<r_model;i++)
			{
				++m_iter;
			}
			vector<bool> ori_curModel=m_iter->second.second;

			//����ԭ��ά�ṹ��������Ͻ����꣬�ж�λ���Ƿ��ܹ���׿�(�����϶��λ��ȫ��Ϊ��)
			//ע�⣺�������귶Χ����Խ��
			int fill_z=rand()%(m_biglayers-m_templesz+1);          //z���귶Χ(0~b_layer-TempSize)
			int fill_y=rand()%(m_bigheight-m_templesz+1);           //y���귶Χ(0~b_rows-TempSize)
			int fill_x=rand()%(m_bigwidth-m_templesz+1);           //x���귶Χ(0~b_cols-TempSize)

			while(!Is_ROI_B(bPore_label,m_bigimg,ori_curModel,fill_z,fill_y,fill_x,m_templesz,m_biglayers,m_bigheight,m_bigwidth,flag01))   //ѡȡλ�ò�����ѡȡ��С��ģ��
			{
				fill_z=rand()%(m_biglayers-m_templesz+1);          
				fill_y=rand()%(m_bigheight-m_templesz+1);
				fill_x=rand()%(m_bigwidth-m_templesz+1);         
			}
			int a=2;
			int b=255;
			int new_label=rand()%(b-a+1)+a;     //Ϊ��ǰ�ؽ���С������2~255�������ǩ(0Ϊͼ�񱳾���1Ϊ��ʼ��ױ�ǩ)

			//�����ѡ��Сģ�鵽�����λ�ã���ͬʱ�ı䵱ǰ��ά�ṹ�Ŀ׵���
			int m=-1;
			int new_poresize=0;     //ͳ�����ؽ���С�����
			for (int z=fill_z;z<(fill_z+m_templesz);z++)
			{
				for (int y=fill_y;y<(fill_y+m_templesz);y++)
				{
					for (int x=fill_x;x<(fill_x+m_templesz);x++)
					{
						++m;
						//180312����
						int t=0;
						if (ori_curModel[m]!=0)
						{
							t=255;
							m_bigimg[z][y][x]=t;
							chg_pNum++;     //������ά�ṹ�ܿ׵�������
							new_poresize++;
							bPore_label[z][y][x]=new_label;     //������С�׿׵��ǩ
						}
					}
				}
			}

			while(b_up==true||b_down==true||b_left==true||b_right==true||b_front==true||b_back==true)      //ĳһ���������ؽ�
			{
				tran++;

				/////////////////////////////////////�ж������ؽ������Ƿ�ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath , chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					new_poreNum++; 
					//cout<<"���ؽ����һ���ף�Ϊ��"<<new_poreNum<<"���ף��ÿ׵����Ϊ"<<new_poresize<<endl;
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


				/////////////////////////////////////�ж������ؽ������Ƿ�ֹͣ/////////////////////////////////////
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//��
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//ǰ
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


				/////////////////////////////////////�ж������ؽ�����ֹͣ/////////////////////////////////////
				if (IsReconstSuccess(savepath, chg_pNum, tar_pNum, deviation, m_bigimg, m_biglayers, m_bigheight, m_bigwidth))     
				{
					return true;
				}
				//��
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
			//cout<<"���ؽ���"<<new_poreNum<<"���ף��ÿ׵����Ϊ"<<new_poresize<<"����ʱ(min)"<<t<<endl;


			//////////////////////////////////���ϸ���&�����ؽ������е���ά����ͼ////////////////////////////////////////
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
        //����ͼƬ����ʽΪ���ֱ�ţ�����1.bmp��
        QString respath = savepath;
        respath.append("/reconstruct/").append(name);
        img.save(respath, nullptr, 100);
      }

		}

	}
	//AfxMessageBox(_T("�ؽ���ɣ�"));
	return true;
}
*/

bool PoreSet::Is_ROI_B(int*** pore_labels, int*** Image3D, vector<bool>& ROI,
                       int z_layers, int y_rows, int x_cols, int ROI_size,
                       int _blayers, int _bRows, int _bCols, int flag01) {
  int e_point = 0;  //������������������ĵ����
  int neighbor_porenum0 = 0;
  // 12-26����¼�����λ�ø��������׵Ŀ׵������жϵ�ǰλ���Ƿ񿿽������ף�
  int neighbor_porenum1 = 0;
  int n = -1;

  for (int z = z_layers; z < (z_layers + ROI_size); z++) {
    for (int y = y_rows; y < (y_rows + ROI_size); y++) {
      for (int x = x_cols; x < (x_cols + ROI_size); x++) {
        ++n;
        if (ROI[n] != 0 && Image3D[z][y][x] != 0)  //�����׵�λ�ò�Ϊ����
        {
          e_point++;
          return false;
        } else  // 12-26�������׵�λ��Ϊ����
        {
          // 12-26���жϵ�ǰ�׵�����򣨴�ֱ6�������2�����ص㳤֮�ڣ������Ƿ��������׵�
          //(12-31�޸ģ���==д����=)
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
  //(12-31�޸ģ���==д����=)
  if ((flag01 == 0 && neighbor_porenum0 != 0) ||
      (flag01 == 1 && neighbor_porenum1 != 0)) {
    return true;
  } else {
    return false;
  }
}


//�ж��ؽ��Ƿ���ɣ�����򱣴��Ӧ��ͼ��
//������
//�ؽ������е�������ά�ṹ�Ŀ׵�����Ŀ��׵������׵������ƫ��ֵ��������ά�׽ṹ�������������ά�ṹ�ģ�������������������
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
    //����ͼƬ����ʽΪ���ֱ�ţ�����1.bmp��
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
	int label_1_num=0;    //ģ���а��������ؽ������ױ�ǩ�ĵ�����

	int reconst_z=fillz+trans*a;    //ƽ�ƺ�ģ�����Ͻǵ�
	int reconst_y=filly+trans*b;
	int reconst_x=fillx+trans*c;

	//��ƽ�ƺ������(10-28�޸ģ�ģ������Ͻ����꣡��)Խ�磡�÷����ϲ��ܼ����ؽ�������false
  if (reconst_z < 0 || reconst_z > (_blayer - m_templesz) ||
      reconst_y < 0 || reconst_y > (_bRows - m_templesz) ||
      reconst_x < 0 || reconst_x > (_bCols - m_templesz)) {
    return false;
  }

	//180311
	//bitset<bs_bits> bs_newmodel;   //Ϊ�˸���Ľ��бȽ�
  vector<bool> bs_newmodel(m_templesz * m_templesz * m_templesz, false);

	string key;
	int n=-1;
	int new_modelpnum=0;    //ƽ�ƺ���ģʽ�Ŀ׵���
	int other_pnum=0;      //ƽ�ƺ���ģʽ�������׵Ŀ׵���

	for (int z=reconst_z;z<(reconst_z+m_templesz);z++)
	{
		for (int y=reconst_y;y<(reconst_y+m_templesz);y++)
		{
			for (int x=reconst_x;x<(reconst_x+m_templesz);x++)
			{
				++n;
				if (_porelabel[z][y][x]!=0/*==_newlabel*/)       //12-25��ֻ�ѵ�ǰ���ؽ��Ŀ׿׵������ģʽ�����ܽ������׵�Ҳ���룡
					                                             //18-1-2:�ѷǱ����㶼���ǽ�����ֻ������ؽ��趨���ܸı��׽ṹ��С�׽ṹ�ɲ��ϸı�
				{
					bs_newmodel[n]=1;
					if (_porelabel[z][y][x]==1)        //18-2-11����ģʽ�д�׽ṹ�Ŀ׵�ĸ���
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

	//����ǰƽ�ƺ��ģ��ȫΪ0���÷����ϲ����ټ���ƥ�䣬����false
	//18-2-11����ƽ�ƺ�ģ��ȫΪ��׽ṹ�Ŀ׵㣬Ҳ����false
	if (new_modelpnum==0||other_pnum==m_templesz*m_templesz*m_templesz)  
	{
		nflag=0;      //12-15��ģ����ȫΪ����
		return false;
	}
	else
	{
		if (m_strkey.find(key)==m_strkey.end())   //���ؽ�ģ����ģʽ���У�
		{
      //ģʽ��Ѱ��������Ŀ�
			int p_max=0;          //�������
			int min_diff=m_templesz*m_templesz*m_templesz+1;       //������С����ֵ����ʼֵ��÷ǳ�����Ҫ��Ϊ�˷��񵱴��ؽ�ģʽ�׵�������ģʽ����ʱ����һ���ҵ���ӽ�����ģʽ�׵���ʱ��ִ��Ѱ�ң�
			//bitset<bs_bits> best_model;     //�洢Ѱ�ҵ������ģ��
      vector<bool> best_model(
          m_templesz * m_templesz * m_templesz, false);

			int n=1;     //��Ҫ�ǽ���һ�����������Ŀ����Ӧ������Ϊmin_diff�ȵĳ�ʼֵ

			////////////////////////////////////11-27��Ѱ����ƥ���ģʽ//////////////////////////////////////

			//��ǰ��������ģʽ�Ŀ׵���
			int cur_modelpnum;       

			stack<int> suit_pnum;        //������ؽ�ģʽ�׵�������С�ڵ���min_diff(��ǰ�ҵ�����Сģʽ��λ����)�Ķ�Ӧ�Ŀ׵�������

			set<int> already_deal;    //�����ѱ����˶�Ӧ��ģʽ���Ŀ׵���
			set<int>::iterator set_iter;

			cur_modelpnum=new_modelpnum;
			already_deal.insert(new_modelpnum);
			IsfindBestModel(bs_newmodel,cur_modelpnum,new_modelpnum,min_diff,p_max,n,suit_pnum,already_deal,best_model);

			while (!suit_pnum.empty())
			{
				cur_modelpnum=suit_pnum.top();
				suit_pnum.pop();     //��������׵�������stack

				set_iter=already_deal.find(cur_modelpnum);

				if (set_iter==already_deal.end())          //��ǰģʽ�׵�����Ӧ��ģʽ��δ������
				{
					int cur_pnum_diff=abs(cur_modelpnum-new_modelpnum);      //���ڿ�����ģʽ��ӵ����ģʽ�׵����У�û���ܹ����ڴ��ؽ��׵����ģ����Ը�ֵ����Ϊ��
					if (cur_pnum_diff<0)
					{
						//AfxMessageBox(_T("abs����ȡ����ֵ����"));
						return false;
					}

					if (cur_pnum_diff<=min_diff)      //����min_diff���ܻ᲻�ϱ�С��������������Խ�һ����С�����ķ�Χ
					{
						already_deal.insert(cur_modelpnum);
						IsfindBestModel(bs_newmodel,cur_modelpnum,new_modelpnum,min_diff,p_max,n,suit_pnum,already_deal,best_model);
					}
				}
			}

			//���ҵ�����ƥ��ģʽ�滻���ؽ�ģʽ
      //12-4����Ѱ�ҵ���ģʽ�Ƿ�������ж���--->12-5�������������㣬����ģʽ����δ�ҵ��׵������ڵ�ǰģʽ��ģʽ����ģʽ�������ؽ�
			if (best_model == vector<bool>(m_templesz*m_templesz*m_templesz,false))            
			{
				/*AfxMessageBox(_T("Ѱ�ҵ������ģʽ��ģʽ���в������ڣ�"));*/
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
						//180312�޸ĵĲ�����Ҫ�������йأ�
						//��ʵ�ʶ��ؽ����������Ӱ�죬ֻ�������˸�ֵ�����ص����
						//����Ϊ�׵�(1)��λ��Ҫ�ٱ���ֵһ��Ϊ(255)��ʵ�ʽ������û���⡣
						if (t!=_3D[z][y][x]         //12-4�޸ģ���!=new_model[k]��Ϊ!=_3D[z][y][x]
						&&_porelabel[z][y][x]!=1)           //12-25�޸ģ�ģʽƥ�䲻�ܸ��ĳ�ʼ�Ĵ�׿׵㣡��׽ṹ���ܸı䣡

						{
							if (t!=0)        //12-26:ƥ����ڵ�ǰ��λ������0->255������->�ף�
							{
								_3D[z][y][x]=t;
								_porelabel[z][y][x]=_newlabel;
								_chgnum++;
								_newporesize++;
							}
							else            //12-26��ƥ���������Ҫ��255->0����->������
							{
								_3D[z][y][x]=t;        //18-1-2:�ǵ�ǰ�ؽ�С�׵�Ҳ�ܱ��ı䣬�������뵱ǰ�ؽ��׵������������ܻ�ó��׳ߴ�Ϊ���Ľ������
								_chgnum--;

								if (_porelabel[z][y][x]==_newlabel)      //12-26��ֻ����ǰ�ؽ��׵Ŀ׵㸳Ϊ����0�������ı������׵�
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

		return true;       //���ÿײ�����������ͨʱ�����ؽ���ģ�鲻����߱仯����
	}

}

bool PoreSet::IsfindBestModel(vector<bool>& bs_newmodel, int& cur_modelpnum,
                              int new_modelpnum, int& min_diff, int& p_max,
                              int& n, stack<int>& suit_pnum,
                              set<int> const& already_deal,
                              vector<bool>& best_m) {
	if (m_st_pnum.find(cur_modelpnum)!=m_st_pnum.end())         //��ģʽ�����ҵ��뵱ǰģʽ�׵�����ͬ����ģʽ������λ����ģʽ��Ѱ����ƥ��ģʽ
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
				//AfxMessageBox(_T("���ڴ����и���bitset�ĳ��ȣ�"));
				return false;
			}

			//�ж�ģʽ����ģʽ���²�����ģʽ��ͬλ�����ص���λ��ͬ��ĸ���
      
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

		//������������ؽ�ģʽ�׵�������ֵС�ڵ��ڵ�ǰ�ҵ�����Сģʽ����λ��min_diff���ж�Ѱ�ҵ���С�����ܷ����ٽ��Ŀ׵���ģʽ�����ҵ������С��һ�������ʸ����ģʽ

		set<int>::const_iterator st_iter=m_st_pnum.begin();       
		for (;st_iter!=m_st_pnum.end();++st_iter)
		{
			int pnum=*st_iter;       //ģʽ�׵���
			//180124��ѡȡ����ӽ���ģʽ����һ��ʹ�׵�������
			int p_diff=abs(pnum-new_modelpnum);           //����ؽ��׵���֮��

			//���׵�������ؽ��׿�׵���֮��С�ڵ��ڵ�ǰ�ҵ�����С��λ��ֵmin_diff�Ҳ�����0����Ϊnew_modelpnum���������������Ŀ׵�������stack
			if (p_diff<=min_diff)
			{
				set<int>::const_iterator ret;
				ret=already_deal.find(pnum);
				if (ret==already_deal.end())       //ֻ��δ����������ģʽ����Ӧ�׵�������stack�У������ظ�����
				{
					suit_pnum.push(pnum);
				}
			}
		
		}
	}
	else          //δ��ģʽ�����ҵ��뵱ǰģʽ�׵�cur_modelpnum��=new_modelpnum��һ�µ���ģʽ����ֻ������new_modelpnumʱ�ſ��ܻ��������������Ϊʣ�µ�suit_pnum�д�Ķ���ģʽ��
	{
		if (!m_st_pnum.empty())
		{
			set<int>::const_iterator st_iter;

			int pnum_diffmin=0;          //�׵�������ֵ��Сֵ
			int best_pnum=0;             //����׵���������С�����ģʽ�׵���(ģʽ����δ����׵���Ϊ0��ģʽ)
			int m=1;

			for (st_iter=m_st_pnum.begin();st_iter!=m_st_pnum.end();st_iter++)     //Ѱ��ģʽ�����뵱ǰģʽ��ӽ��Ŀ׵���(���������׵�������һ���ӽ�����ѡ��Ƚϴ���Ǹ���)
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
					break;          //����set�ǰ�valueֵ��С����洢�ģ�diffֵӦ�ó����𽥼�С������ӽ���ģʽ�׵����ﵽ��Сֵ���������ӣ�
				}
		
			}

			suit_pnum.push(best_pnum);
			
		}

	}
	return true;
}
