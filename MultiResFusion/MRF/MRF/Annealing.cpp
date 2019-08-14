#include "stdafx.h"
#include "io.h"
#include "Annealing.h"
#include  <vector>
#include <fstream>
#include <atlstr.h>
using namespace std;
using namespace cv;

   //ʹ��CFileDialog���ͷ�ļ�

CAnnealing::CAnnealing(void)
{
}

CAnnealing::~CAnnealing(void)
{
}

//size_number(ģ���С��Ӧ��������ֵ)��ʼ��
//Rec_template_size=3
//Inital_image_vector���ԭͼ������
//big_3Dimg��Ŵ����ά�ṹ
//beishu��ʼ��
CAnnealing::CAnnealing(IplImage* img, int width, int height,int template_size)
{
	image = new uchar[width*height];
	class_white_point_number=0;
	class_black_point_number=0;
	grid_size=0;
	vector_size=0;
	inital_height=height;              //������ԭͼ�ĳߴ縳�������������������еĺ���ʹ��
	inital_width=width;                //������ԭͼ�ĳߴ縳�������������������еĺ���ʹ�ã�
	Rec_template_size=template_size;   //�����������ݲ���ģ���С������
    size_number=(int)pow(2.0,pow(Rec_template_size,2.0));
	Enerry_origial=0;;
	Enerry_Reverse=0;

	int i=0,j=0,k=0;
	Inital_image_vector.resize(width);
	for (j=0;j!=width;j++)
	{
		Inital_image_vector[j].resize(height);
		for (i=0;i!=height;i++)
		{
			k= img->imageData[j*width+i];
			if (k==0)
			{
				Inital_image_vector[j][i]=0;
			}
			else
			{
				Inital_image_vector[j][i]=1;
			}
		}
	}
	/////////////////////////////190113/////////////////////////////////////
	cout<<"�����������ؽ���ͼ��ߴ�(��ѵ��ͼ��ߴ����������ϵ)��"<<endl;
	cin>>final_imgsize;
	if (final_imgsize<height)
	{
		cout<<"�ؽ��ߴ�С��ѵ��ͼ��"<<endl;
		return;
	}
	beishu=final_imgsize/height;     //������ؽ��ߴ���ѵ��ͼ��֮��ı�����ϵ

	//////////////////////////190211������ͷֱ��ʴ����ά�ṹ////////////////////////////
	int b_layer;
	cout<<"��������(�ͷֱ���)��ά�ṹ�Ĳ�����"<<endl;
	cin>>b_layer;

	if (final_imgsize!=b_layer)
	{
		cout<<"С���ںϳߴ����׽ṹ�ߴ粻��ȣ�"<<endl;
		return;
	}

	big_3Dimg.resize(b_layer);
	AfxSetResourceHandle(GetModuleHandle(NULL));        //�ڿ���̨������ʹ��CFileDialog�����Ҫ���
	AfxMessageBox(_T("��򿪴��(�ͷֱ���)��ά�ṹ���ڰס�����ͼ�����ļ��У�"));
	CFileDialog dlgOpen(TRUE);
	if (dlgOpen.DoModal()==IDOK)
	{
		CString cOpenPath=dlgOpen.GetFolderPath();
		CString cpicType=dlgOpen.GetFileExt();
		string OpenPath=cOpenPath.GetBuffer(0);
		string picType=cpicType.GetBuffer(0);

		string picPath=OpenPath+"\\"+"*."+picType;
		vector<string>pic_list;
		
		glob(picPath, pic_list,false);
		//get_filelist_from_dir(picPath,pic_list);

		if (pic_list.size()==0)
		{
			AfxMessageBox(_T("�ļ���ȡ����"));
		}
		
		//��������ͼ������
		for (int z=0;z<b_layer;z++)
		{
			string img_pth=/*OpenPath+"\\"+*/pic_list[z];
			Mat b_img=cv::imread(img_pth,CV_LOAD_IMAGE_GRAYSCALE);
			int b_rows=b_img.rows;
			int b_cols=b_img.cols;
		
			big_3Dimg[z].resize(b_rows);

			for (int y=0;y<b_rows;y++)
			{
				big_3Dimg[z][y].resize(b_cols);
				uchar* p=b_img.ptr<uchar>(y);
				for (int x=0;x<b_cols;x++)
				{
					int t=0;         //��ֹ�����ͼ��׵㲻�ǰ�ɫ��
					if (p[x]!=0)
					{
						t=255;
					}
					big_3Dimg[z][y][x]=t;
				}
			}
		}
	}
	////����Ϊ�˲���
	//for (int y = 0; y < 256; y++)
	//{
	//	for (int x = 0; x < 256; x++)
	//	{
	//		cout << big_3Dimg[1][y][x] << endl;
	//	}
	//}
	////cout << "save successfully" << endl;
}

/////////////////////////////////1��ȷ����С����/////////////////////////////////////////
void CAnnealing::Decide_min_grid(int& min_grid)
{
	int i=0,j=0,p=0,q=0,sum=0,k=0,w=0;
	int white_point=0,black_point=0;
	int loop_time=log((float)inital_width)/log(2.0);
	int temporay_value_1=0,temporay_value_2=0;
	bool judgement=false;

	for (k=0;k!=loop_time-1;k++)////��С�����4*4��ʼ
	{
		temporay_value_1=(int)pow(2.0,k);                   ////���������֮��ľ���
		temporay_value_2=inital_width/temporay_value_1;     ////����Ĵ�С
		white_point=0;
		black_point=0;
		judgement=false;
		for (i=0;i!=temporay_value_2;i++)
		{
			for (j=0;j!=temporay_value_2;j++)
			{
				if (Inital_image_vector[i*temporay_value_1][j*temporay_value_1]==1)
				{
					white_point++;
				}
				else
				{
					black_point++;
				}
			}
		}
		for (i=1;i!=temporay_value_2-1;i++)      //3*3ģ������ĵ㣬�жϷּ�ͼ���Ƿ����3*3����Ϊͬһ�ֻ࣬Ҫ���ڣ������Ѱ��
		{
			for (j=1;j!=temporay_value_2-1;j++)
			{
				sum=0;
				for (p=-1;p!=2;p++)
				{
					for (q=-1;q!=2;q++)
					{
						sum=sum+Inital_image_vector[(i+p)*temporay_value_1][(j+q)*temporay_value_1];
					}
				}
				if (sum==9 /*|| sum==0*/)
				{
					judgement=true;
				}
			}
		}
		if (judgement==false)
		{
			break;
		}
	}
	min_grid=temporay_value_2;
}

void CAnnealing::Transport_min_grid(int min_grid)
{
	Rec_min_grid_size=min_grid;
}

inline unsigned __int32 rand32()
{
	return ((rand()&0x00007FE0)>>5) + ((rand()&0x00007FF0)<<6) + ((rand()&0x00007FF0)<<17);
}

//inital_rand_data(��ʼ��С����)��ʼ��(190113��)->(190212���������ά�ṹ��ϢҲ���浽��С������)
//��׿׵�����λ�ó�ʼ��Ϊ300������190212�����ؽ��ı���������ڴ�׿׵����ڵ�λ�ã��������ܱ�֤�ؽ�����Ŀ�϶����ȷ���ұ�֤��׽ṹ����Ӱ��
//��׿׵�λ��ֻ������ΪС�׿׵㲻���ؽ�����������С�ױ����㣨��׿׵�̶������ܱ������ֵ��
void CAnnealing::Set_inital_data()
{
	//�ȶ���һ����ά���������ڴ�ų�ʼ��С����
	int i=0,j=0,k=0,inital_white_point=0;

	int new_inital_size=Rec_min_grid_size*beishu;     //190113
	int size_number=inital_height/Rec_min_grid_size;   
	int b_pnum=0;   //��׿׵���                      //190212

	//inital_rand_data��ʼ��Ϊ2��300��֮ǰȫ��2��
	inital_rand_data.resize(new_inital_size);      //inital_rand_data��ʼ��С���������ֵ��
	for (i=0;i!=new_inital_size;++i)
	{
		inital_rand_data[i].resize(new_inital_size);
		for (j=0;j!=new_inital_size;++j)
		{
			inital_rand_data[i][j].resize(new_inital_size);    
			for (k=0;k!=new_inital_size;k++)
			{
				if (big_3Dimg[i*size_number][j*size_number][k*size_number]!=0)
				{
					inital_rand_data[i][j][k]=300;      //190212����׿׵�����λ��
					++b_pnum;
				}
				else
					inital_rand_data[i][j][k]=2;
			}
		}
	}
	//���տ�϶�Ƚ����������ֵ����ɫ��1��ʾ����ɫ��0��ʾ
	for (i=0;i!=Rec_min_grid_size;i++)
	{
		for (j=0;j!=Rec_min_grid_size;j++)
		{
			if (Inital_image_vector[i*size_number][j*size_number]==1)    //Inital_image_vector���ԭͼ�������߷ֱ棩
			{
				inital_white_point++;
			}
		}
	}
	int rand_site=0;
	int counter_number=new_inital_size*new_inital_size*new_inital_size/*-b_pnum*/;  //190212
	vector<int> point_x;
	vector<int> point_y;
	vector<int> point_z;
	vector<int>::iterator site_x=point_x.begin();
	vector<int>::iterator site_y=point_y.begin();
	vector<int>::iterator site_z=point_z.begin();
	for (i=0;i!=new_inital_size;i++)    //190113
	{
		for(j=0;j!=new_inital_size;j++)
		{
			for (k=0;k!=new_inital_size;k++)
			{
				//if (inital_rand_data[i][j][k]!=300)    //190212
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
	for (i=0;i!=new_inital_size*inital_white_point*(beishu*beishu);i++)   
	{
		rand_site= rand32()%counter_number;
		if (inital_rand_data[point_x[rand_site]][point_y[rand_site]][point_z[rand_site]]!=300)   //190212
		{
			inital_rand_data[point_x[rand_site]][point_y[rand_site]][point_z[rand_site]]=1;//1��ʾ��ɫ�㣬0��ʾ��ɫ��
			site_x= point_x.begin()+rand_site;   //�ҵ��Ѿ������ֵ�ĵ㣬��ɾ���������ظ���ֵ
			site_y= point_y.begin()+rand_site;
			site_z= point_z.begin()+rand_site;
			point_x.erase(site_x);
			point_y.erase(site_y);
			point_z.erase(site_z);
			counter_number--;
		}
		else
		{
			--i;
			site_x= point_x.begin()+rand_site;   //190213���ҵ�����׿׵㡱����λ�ã���ɾ���������ظ���ֵ
			site_y= point_y.begin()+rand_site;
			site_z= point_z.begin()+rand_site;
			point_x.erase(site_x);
			point_y.erase(site_y);
			point_z.erase(site_z);
			counter_number--;
		}
	}
	point_x.clear();
	point_y.clear();
	point_z.clear();
	for (i=0;i!=new_inital_size;i++)
	{
		for (j=0;j!=new_inital_size;j++)
		{
			for (k=0;k!=new_inital_size;k++)
			{
				if (inital_rand_data[i][j][k]!=1/*&&inital_rand_data[i][j][k]!=300*/)   //190212
				{
					inital_rand_data[i][j][k]=0;
				}
			}
		}
	}
}

//Ϊ(new)vector_size(��ǰ��ѵ��ͼ��ĳߴ�)��ֵ
void CAnnealing::Transfer_grid_size(int T_grid_size)
{
	grid_size=T_grid_size;
	vector_size=Rec_min_grid_size*(int)pow(2.0,grid_size);
	new_vector_size=vector_size*beishu;   //190113
}

//TI_code_value(���ѵ��ͼ��Ķ���ܶȺ���)
void CAnnealing::TI_Multi_point_density_function_p()
{
	TI_code_value.clear();   //TI_code_value���ѵ��ͼ��Ķ���ܶȺ���             //����һ����洢�Ķ���ܶȺ������
	int i=0,j=0;
	int reduce_size=inital_height/vector_size;//��С����
	int k=Rec_template_size/2;
	vector<vector<int>> Temporary_vector;
	Temporary_vector.resize(vector_size);    //���浱ǰ������TI
	for(i=0;i!=vector_size;++i)
	{
		Temporary_vector[i].resize(vector_size);
	}

	//����Ӧ���ԭʼͼ��ŵ��м��������
	for (i=0;i!=vector_size;i++)
	{
		for (j=0;j!=vector_size;j++)
		{
			if (Inital_image_vector[i*reduce_size][j*reduce_size]==1)
			{
				Temporary_vector[i][j]=1;//Ϊ�˺�����뷽�㽫255ֵ��Ϊ1
			}
			else
			{
				Temporary_vector[i][j]=0;
			}
		}
	}

	//�����ʱ��Сͼ��Ķ���ܶȺ���
	int code_value=0;//ģʽ����ֵ
	int q=0,p=0;
	for (i=0;i!=vector_size;++i)
	{
		for (j=0;j!=vector_size;++j)
		{
			code_value=0;
			for (p=-k;p!=k+1;p++)
			{
				for (q=-k;q!=k+1;q++)
				{
					//���ֱ��뷽ʽ�ܹ���֤���벻�ظ���������ֵΪ511
					code_value=code_value*2+Temporary_vector[(i+p+vector_size)%vector_size][(j+q+vector_size)%vector_size];
				}
			}
			pair<map<int,int>::iterator,bool> pair_code_value=TI_code_value.insert(make_pair(code_value,1));
			if(!pair_code_value.second)           
			{
				++pair_code_value.first->second; 
			}
		}
	}
}

void CAnnealing::Putout_inital_image()
{
	IplImage* pImage=cvCreateImage(cvSize(vector_size,vector_size), IPL_DEPTH_8U, 1);//�洢ͼƬ	
	int step =pImage->widthStep;  //ÿ��ͼ��ÿ���ֽ���
	uchar *data=(uchar *)pImage->imageData;
	int i=0,j=0,k=0,q=0,white_point=0;
	k=inital_height/(vector_size);
	for (i=0;i!=vector_size;++i)
	{
		for (j=0;j!=vector_size;++j)
		{
			q=Inital_image_vector[i*k][j*k];
			if (q==1)
			{
				q=255;
			}
			pImage->imageData[i*step+j]=q;
			//if (q==255)
			//{
			//	white_point++;
			//}
		}
	}
	//cout<<"ԭʼ��ɫ����:"<<white_point<<endl;
	char name[100];
	sprintf(name,"The inital%d!.bmp",grid_size);   //grid_size����T_grid_size

	cvSaveImage(name, pImage);
	cvReleaseImage(&pImage);
}

//190212��reconstruct_vector��һ�γ�ʼ��ʱ��׿׵�����λ�ö���ʼ��Ϊ0�ˣ�
void CAnnealing::Set_inital_to_reconstruct_uncondition_1()    //Ϊ��С�����Ӧ����ά�ṹ����ֵ����ΪSet_inital_data��
{
	int i=0,j=0,k=0;
	reconstruct_vector.resize(new_vector_size);
	for(i=0;i!=new_vector_size;++i)
	{
		reconstruct_vector[i].resize(new_vector_size);
		for (j=0;j!=new_vector_size;j++)
		{
			reconstruct_vector[i][j].resize(new_vector_size);
			for (k=0;k!=new_vector_size;k++)
			{
				reconstruct_vector[i][j][k]=inital_rand_data[i][j][k];
			}
		}
	}
}

//Pre_process_vector
void CAnnealing::Set_inital_to_reconstruct_uncondition_2()
{
	int i=0,j=0,k=0,p=0,q=0,n=0;
	int sum=0;
	int up_grid_white_number=0;
	int current_grid_white_number=0;
	int all_add_white_number=0;
	int all_add_black_number=0;
	int add_white_number_3D=0;
	int add_white_number_2D=0;
	//int add_white_number_1D=0;
	int white_point_number=0;
	int distance_point=inital_width/vector_size; 
	/////////////////////////////////////////////////���о���������ֵ�����ϲ�ͼ�񴫵ݸ��ؽ�������
	reconstruct_vector.resize(new_vector_size);     //(190113)����ؽ��������
	Pre_process_vector.resize(new_vector_size);     //���Ԥ��������������ע3*3ȫΪ�ڻ���ȫ�׵����� 
	for (i=0;i!=new_vector_size;++i)
	{
		reconstruct_vector[i].resize(new_vector_size);
		Pre_process_vector[i].resize(new_vector_size);
		for (j=0;j!=new_vector_size;j++)
		{
			reconstruct_vector[i][j].resize(new_vector_size);
			Pre_process_vector[i][j].resize(new_vector_size);
			for (k=0;k!=new_vector_size;k++)
			{
				if (big_3Dimg[i*distance_point][j*distance_point][k*distance_point]!=0)
				{
					Pre_process_vector[i][j][k]=300;                              /////190212
					reconstruct_vector[i][j][k]=2;                            /////��ʼ���м�����"��׿׵�"Ϊ300
				}
				else
				{
					Pre_process_vector[i][j][k]=2;                              /////��ʼ���м�����Ϊ2
					reconstruct_vector[i][j][k]=2;                              /////��ʼ���м�����Ϊ2
				}
			}
		}
	}
	for (i=0;i!=vector_size;++i)
	{
		for (j=0;j!=vector_size;++j)
		{                             
			if (Inital_image_vector[i*distance_point][j*distance_point]==1)              /////���㱾��ԭͼ��ɫ����
			{
				current_grid_white_number++;
			}
		}
	}
	////////////////

	for (i=0;i!=new_vector_size/2;++i)   //190113
	{
		for (j=0;j!=new_vector_size/2;++j)
		{
			for (k=0;k!=new_vector_size/2;++k)
			{
				///////190212
				if (Pre_process_vector[2*i][2*j][2*k]!=300)       
				{
					reconstruct_vector[2*i][2*j][2*k]=inital_rand_data[i][j][k];              /////���ϲ��ؽ��Ľ�����ݵ���Ӧ�����
					Pre_process_vector[2*i][2*j][2*k]=inital_rand_data[i][j][k];              /////���ϲ��ؽ��Ľ�����ݵ���Ӧ�����
					if (inital_rand_data[i][j][k]==1)
					{
						up_grid_white_number++;                                               /////�����ϲ��ؽ��Ľ����϶�ࣨ190212�������²��׿׵��غϣ��ĸ���
					}
				}
				else
				{
					//if (inital_rand_data[i][j][k]==0)    //190213����׿׵����ڵ�λ��ֵҲΪ0��ȥ����һ����������׿�
					//{                                             //������λ����Ϊ��֪�ı��������㣬�Ա�ʵ��������
						reconstruct_vector[2*i][2*j][2*k]=0;                                      
					/*}*/
				}
			}
		}
	}

	/////////////////////////////////////////////////////
	for (i=0;i!=new_vector_size;++i)    //190113����������x��y��z��Ӧ�����ص㣡��
	{
		for (j=0;j!=new_vector_size;++j)
		{
			for (k=0;k!=new_vector_size;++k)
			{
				if (i%2==1 && j%2==1 && k%2==1)    //3*3ģ�����ĵ㳤������궼����Ϊ������
				{
					sum=0;                                                                    /////�ж�3*3*3�����Ƿ�ȫΪ1����ȫ0
					for (q=-1;q!=2;q++)    //i,j,kΪ��������+��=ż����+ż=�棩
					{
						for (p=-1;p!=2;p++)
						{
							for (n=-1;n!=2;n++)    
							{
								//�����������Ҫ���й�ʽ��1��Ϊż����(i+q+new_vector_size)%new_vector_sizeʹ�õĲ��������ڱ߽���������ֹԽ�磡
								if (((i+q+new_vector_size)%new_vector_size)%2==0 && ((j+p+new_vector_size)%new_vector_size)%2==0 && ((k+n+new_vector_size)%new_vector_size)%2==0)
								{
									//q��p��n��ҪΪ����
									//������������(-1/1,-1/1,-1/1)��8���㣬���Ǹպ�Ϊǰһ��ӳ������ĵ�
									sum=sum+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+n+new_vector_size)%new_vector_size];
								}
							}
						}
					}
					//ע�⣺reconstruct_vector���ʼ��ֵΪ2������һ���ֵ㣨�����ֵ0��2��4...����ֵ�ָ���ǰһ���ؽ��������ʼ��Ϊ0��1
					//����sum��ͳ�Ƶ�ǰһ��ӳ����������ص㣡
					if (sum==8)                          /////ȫ1ʱ�������������ĵ��⣬�����ȫ��ֵΪ1
					{
						for (q=-1;q!=2;q++)               //i,j,kΪ��������+��=ż����+ż=�棩
						{
							for (p=-1;p!=2;p++)
							{
								for (n=-1;n!=2;n++)
								{
									//��һ����ʽ�����Ϊ��������
									if (((i+q+new_vector_size)%new_vector_size)%2==1 || ((j+p+new_vector_size)%new_vector_size)%2==1 || ((k+n+new_vector_size)%new_vector_size)%2==1)
									{
										//q��p��n������һ��Ϊż������0��
										//��������ǰһ��ӳ������ĵ�ȫΪ1ʱ��3*3*3ģ��������ֵΪ2�ĵ㣨��չ�㣩
										if (Pre_process_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+n+new_vector_size)%new_vector_size]!=300)   //190212
										{
											Pre_process_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+n+new_vector_size)%new_vector_size]=31;
										}
									}
								}
							}
						}
					}
					else if (sum==0)  /////ȫ0ʱ�������������ĵ��⣬�����ȫ��ֵΪ0
					{
						for (q=-1;q!=2;q++)
						{
							for (p=-1;p!=2;p++)
							{
								for (n=-1;n!=2;n++)
								{
									if (((i+q+new_vector_size)%new_vector_size)%2==1 || ((j+p+new_vector_size)%new_vector_size)%2==1 || ((k+n+new_vector_size)%new_vector_size)%2==1)
									{
										//if (Pre_process_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+n+new_vector_size)%new_vector_size]!=300)    //190212�����Բ�Ҫ��������ɣ���׿׵������С�׽ṹ�ı�����
										//{
											Pre_process_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+n+new_vector_size)%new_vector_size]=30;     
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
	//current_grid_white_numberΪ����ѵ��ͼ���еİ׵���(190113)��up_grid_white_numberΪǰһ���ؽ�����еİ׵���
	all_add_white_number=new_vector_size*current_grid_white_number*(beishu*beishu)-up_grid_white_number;         /////����������Ҫ��Ӱ�ɫ�����
	all_add_black_number=new_vector_size*(new_vector_size*new_vector_size-current_grid_white_number*(beishu*beishu));
	int white_point_number_3D=0;
	int black_point_number_3D=0;
	int white_point_number_2D=0;
	int black_point_number_2D=0;
	//int white_point_number_1D=0;
	//int black_point_number_1D=0;

	for (i=0;i!=new_vector_size;++i)         /////ͳ���������1��0�ĸ���
	{
		for (j=0;j!=new_vector_size;++j)
		{
			for (k=0;k!=new_vector_size;++k)
			{
				//Pre_process_vector��Ϊ�ڰ׵���ܴ��ڵ�λ��
				//������ľۺϹ��ɣ�ǰһ��ȫΪ�׵�/�ڵ��ģʽ������һ������չģʽ����չ������Ҳ�ǰ׵�/�ڵ�
				if (Pre_process_vector[i][j][k]==31)           //3*3*3����ȫΪ1
				{
					white_point_number_3D++;
				}
				else if (Pre_process_vector[i][j][k]==30)      //3*3*3����ȫΪ0
				{
					black_point_number_3D++;
				}
			}
		}
	}

	int point_x=0;
	int point_y=0;
	int point_z=0;

	//��ʵ�ʴ���ӵĺڰ׵���>=���ݾۺϹ��ɼ���ĺڰ׵�������ֱ�ӽ�����ĺڰ׵�λ��ֱ�Ӱ����踳ֵ
	if (all_add_white_number>=white_point_number_3D && all_add_black_number>=black_point_number_3D) /////��reconstruct_vector��Ӧ3D����㸳ֵ1��0
	{
		for (i=0;i!=new_vector_size;++i)//�Ƚ�3*3*3�����жϵĵ�ֱ�ӽ���Щ�������������ݸ�ֵ
		{
			for (j=0;j!=new_vector_size;++j)
			{
				for (k=0;k!=new_vector_size;++k)
				{
					if (Pre_process_vector[i][j][k]==31)    
					{
						reconstruct_vector[i][j][k]=1;
					}
					else if (Pre_process_vector[i][j][k]==30)
					{
						reconstruct_vector[i][j][k]=0;
					}
				}
			}
		}
		//���棺����ż����
		////////////////////////////////////////////////////////////////////////////���ϲ����Ǵ����ϼ�����3*3*3����Ϊȫ1��ȫ0���
		////////////////////////////////////////////////////////////////////////////���²����Ǵ����ϼ�����2*2����Ϊȫ1��ȫ0���
		for (i=0;i!=new_vector_size;++i)//�ж�ȫ1����ȫ0��2*2*2��������
		{
			for (j=0;j!=new_vector_size;++j)
			{
				for (k=0;k!=new_vector_size;++k)
				{
					//iΪż����j��kΪ�����������ص㻹δ��ֵ�����ǳ�ʼֵ2||(190212��300)��
					if (i%2==0 && j%2==1 && k%2==1 && (Pre_process_vector[i][j][k]==2||Pre_process_vector[i][j][k]==300) )
					{
						sum=0;
						for (q=-1;q!=2;q++)     
						{
							for (p=-1;p!=2;p++)
							{
								//ֻ����j��k����Ҫq��pΪ����
								if (((j+q+new_vector_size)%new_vector_size)%2==0 && ((k+p+new_vector_size)%new_vector_size)%2==0)
								{
									sum=sum+reconstruct_vector[i][(j+q+new_vector_size)%new_vector_size][(k+p+new_vector_size)%new_vector_size];
								}
							}
						}
						if (sum==4)
						{
							if (Pre_process_vector[i][j][k]!=300)    //190213���Ƿ��е����⣿Pre������ڵ�λ�ÿ�������һ�����Ѿ���ֵΪ30����
							{                                                  //û���⣡��Ϊ���������Pre��ֵֻ��Ϊ2��300��
								Pre_process_vector[i][j][k]=21;    
								white_point_number_2D++;
							}
						}
						else if (sum==0)
						{
							Pre_process_vector[i][j][k]=20;
							black_point_number_2D++;
						}
					}

					//kΪż����j��iΪ�����������ص㻹δ��ֵ�����ǳ�ʼֵ2||(190212��300)��
					else if (i%2==1 && j%2==1 && k%2==0 && (Pre_process_vector[i][j][k]==2||Pre_process_vector[i][j][k]==300) )
					{
						sum=0;
						for (q=-1;q!=2;q++)
						{
							for (p=-1;p!=2;p++)
							{
								if (((i+q+new_vector_size)%new_vector_size)%2==0 && ((j+p+new_vector_size)%new_vector_size)%2==0)
								{
									sum=sum+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][k];
								}
							}
						}
						if (sum==4)
						{
							if (Pre_process_vector[i][j][k]!=300)    //190212
							{
								Pre_process_vector[i][j][k]=21;
								white_point_number_2D++;
							}
						}
						else if (sum==0)
						{
							Pre_process_vector[i][j][k]=20;
							black_point_number_2D++;
						}
					}

					//jΪż����k��iΪ�����������ص㻹δ��ֵ�����ǳ�ʼֵ2||(190212��300)��
					else if (i%2==1 && j%2==0 && k%2==1 && (Pre_process_vector[i][j][k]==2||Pre_process_vector[i][j][k]==300) )
					{
						sum=0;
						for (q=-1;q!=2;q++)
						{
							for (p=-1;p!=2;p++)
							{
								if (((i+q+new_vector_size)%new_vector_size)%2==0 && ((k+p+new_vector_size)%new_vector_size)%2==0)
								{
									sum=sum+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][j][(k+p+new_vector_size)%new_vector_size];
								}
							}
						}
						if (sum==4)
						{
						    if (Pre_process_vector[i][j][k]!=300)    //190212
							{
								Pre_process_vector[i][j][k]=21;
								white_point_number_2D++;
							}

						}
						else if (sum==0)
						{
							Pre_process_vector[i][j][k]=20;
							black_point_number_2D++;
						}
					}
				}
			}
		}
		/////�ڶ���2*2���ĵ�ֱ�Ӹ�ֵ�����϶��
		if (all_add_white_number>=(white_point_number_3D+white_point_number_2D) && all_add_black_number>=(black_point_number_3D+black_point_number_2D))
		{
			for (i=0;i!=new_vector_size;++i)//��ֱ�ӽ���Щ�������������ݸ�ֵ
			{
				for (j=0;j!=new_vector_size;++j)
				{
					for (k=0;k!=new_vector_size;++k)
					{
						if (Pre_process_vector[i][j][k]==21)    
						{
							reconstruct_vector[i][j][k]=1;
						}
						else if (Pre_process_vector[i][j][k]==20)
						{
							reconstruct_vector[i][j][k]=0;
						}
					}
				}
			}

			///////////////�������ж�2*2�����Ƿ�ȫΪ0����1
			add_white_number_2D=(all_add_white_number-white_point_number_3D-white_point_number_2D);//Ȼ���ʣ�ಿ�������ֵ
			while (add_white_number_2D!=0)
			{
				point_x=rand32()%new_vector_size;
				point_y=rand32()%new_vector_size;
				point_z=rand32()%new_vector_size;
				if ((point_y%2==1 || point_x%2==1 || point_z%2==1) && Pre_process_vector[point_x][point_y][point_z]==2 && reconstruct_vector[point_x][point_y][point_z]==2)
				{
					reconstruct_vector[point_x][point_y][point_z]=1;
					add_white_number_2D--;
				}
			}

			for (i=0;i!=new_vector_size;i++)
			{
				for(j=0;j!=new_vector_size;j++)
				{
					for (k=0;k!=new_vector_size;k++)
					{
						if (reconstruct_vector[i][j][k]==2)
						{
							reconstruct_vector[i][j][k]=0;
						}
					}
				}
			}
		}

		else /////�ڶ���2*2*2���ĵ�ֱ�Ӹ�ֵ�������϶��ʱ�����˵���һ��3*3*3����״̬
		{
			cout<<"Unexpected condition second!!"<<endl;
			add_white_number_3D=all_add_white_number-white_point_number_3D;
			while (add_white_number_3D!=0)
			{
				point_x=rand32()%new_vector_size;
				point_y=rand32()%new_vector_size;
				point_z=rand32()%new_vector_size;
				if ((point_y%2==1 || point_x%2==1 || point_z%2==1) && Pre_process_vector[point_x][point_y][point_z]==2)
				{
					reconstruct_vector[point_x][point_y][point_z]=1;
					add_white_number_3D--;
				}
			}
			for (i=0;i!=new_vector_size;i++)
			{
				for(j=0;j!=new_vector_size;j++)
				{
					for (k=0;k!=new_vector_size;k++)
					{
						if (reconstruct_vector[i][j][k]==2)
						{
							reconstruct_vector[i][j][k]=0;
						}
					}
				}
			}
		}
	}

	else   /////ֱ�Ӱ�����Ҫ�����ֵ
	{
		cout<<"Unexpected condition first!!"<<endl;
		while (all_add_white_number!=0)
		{
			point_x=rand32()%new_vector_size;
			point_y=rand32()%new_vector_size;
			point_z=rand32()%new_vector_size;
			if ((point_y%2==1 || point_x%2==1 || point_z%2==1)  && Pre_process_vector[point_x][point_y][point_z]!=300)   //190212
			{
				reconstruct_vector[point_x][point_y][point_z]=1;
				all_add_white_number--;
			}
		}
		for (i=0;i!=new_vector_size;i++)
		{
			for(j=0;j!=new_vector_size;j++)
			{
				for (k=0;k!=new_vector_size;k++)
				{
					if (reconstruct_vector[i][j][k]==2)
					{
						reconstruct_vector[i][j][k]=0;
					}
					Pre_process_vector[i][j][k]=reconstruct_vector[i][j][k];/////���Pre_process_vector��Ϊָʾ����
				}
			}
		}
	}
	/////��Pre_process_vector����Ϊָʾ��
	for (i=0;i!=new_vector_size;i++)
	{
		for(j=0;j!=new_vector_size;j++)
		{
			for (k=0;k!=new_vector_size;k++)
			{
				if (Pre_process_vector[i][j][k]!=2 && Pre_process_vector[i][j][k]!=300 && reconstruct_vector[i][j][k]==1)
				{                                     //190213��������������Ҫ����ΪreconstΪ1��λ��һ�����Ǵ�׿׵�λ��
					Pre_process_vector[i][j][k]=1;
				}
				else if (Pre_process_vector[i][j][k]!=2 && reconstruct_vector[i][j][k]==0)
				{
					Pre_process_vector[i][j][k]=0;
				}
			}
		}
	}
}

//Reconstruct_code_value(����ؽ�ͼ��Ķ���ܶȺ���)����190212��reconstruct_vector��û��300ֵ��
void CAnnealing::Recstruct_Multi_point_density_function_p()
{
	Reconstruct_code_value.clear();//����һ����洢�Ķ���ܶȺ������  
	int i=0,j=0,k=0,q=0,p=0,r=0;
	int s=Rec_template_size/2;
	pair<map<int,int>::iterator,bool> pair_code_value;//�����ʱ�ؽ�ͼ��Ķ���ܶȺ���
	int code_value=0;//ģʽ����ֵ
	for (k=0;k!=new_vector_size;++k)  //���������ģʽ�ܶȽ���ͳ��
	{
		for (j=0;j!=new_vector_size;++j)
		{
			for (i=0;i!=new_vector_size;++i)
			{
				code_value=0;
				for (p=-s;p!=s+1;p++)
				{
					for (q=-s;q!=s+1;q++)
					{
						code_value=code_value*2+reconstruct_vector[k][(i+p+new_vector_size)%new_vector_size][(j+q+new_vector_size)%new_vector_size];
					}
				}
				pair_code_value=Reconstruct_code_value.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           
				{
					++pair_code_value.first->second; 
				}
				code_value=0;
				for (p=-s;p!=s+1;p++)
				{
					for (q=-s;q!=s+1;q++)
					{
						code_value=code_value*2+reconstruct_vector[(i+p+new_vector_size)%new_vector_size][k][(j+q+new_vector_size)%new_vector_size];
					}
				}
				pair_code_value=Reconstruct_code_value.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           
				{
					++pair_code_value.first->second; 
				}
				code_value=0;
				for (p=-s;p!=s+1;p++)
				{
					for (q=-s;q!=s+1;q++)
					{
						code_value=code_value*2+reconstruct_vector[(i+p+new_vector_size)%new_vector_size][(j+q+new_vector_size)%new_vector_size][k];
					}
				}
				pair_code_value=Reconstruct_code_value.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           
				{
					++pair_code_value.first->second; 
				}
			}
		}
	}	
}

void CAnnealing::Calculate_E_differ(int& E_differ)
{
	int i=0;
	E_differ=0;
	for (i=0;i!=size_number;i++) //size_numberģ���Ӧ��С�������ֵ
	{
		//(190113)3��ʾx��y��z�����棬vector_sizeΪ��ά������TI_code_valueΪ�����ģʽ�ܶȺ���
		E_differ=E_differ+abs(3*new_vector_size*TI_code_value[i]*(beishu*beishu)-Reconstruct_code_value[i]);
	}
	Enerry_origial=E_differ;
}

//white_point_number=Main_white_point_number
//190213������reconstruct_vector��û�жԴ�׿׵�ı�־(300)�������޶���׿׵�λ��ֻ��Ϊ�������Ҳ��ܲ��뽻����
void CAnnealing::Select_inital_exchange_point_uncondition(int& white_point_number,int& black_point_number)
{
	int i=0,j=0,k=0;
	int p=0,q=0,g=0;
	white_point_number=0;
	black_point_number=0;

	int distance_point=inital_width/vector_size;     //190213

	for (i=0;i!=new_vector_size;i++)
	{
		for (j=0;j!=new_vector_size;j++)
		{
			for (k=0;k!=new_vector_size;k++)
			{
				if (reconstruct_vector[i][j][k]==1)    //����ؽ��������
				{
					white_point_x.push_back(i);
					white_point_y.push_back(j);
					white_point_z.push_back(k);
					++white_point_number;
				}
				else if (big_3Dimg[i*distance_point][j*distance_point][k*distance_point]==0 && reconstruct_vector[i][j][k]==0)
				{                  //190213��ֻ�зǴ�׿׵�����λ�õı�������ܲ��뽻����
					black_point_x.push_back(i);
					black_point_y.push_back(j);
					black_point_z.push_back(k);
					++black_point_number;
				}
			}
		}
	}
	class_white_point_number=white_point_number;
	class_black_point_number=black_point_number;
}

//temporay_vector���ѡ�񽻻���
void CAnnealing::Select_final_exchange_point_uncondition(int& white_point_number,int& black_point_number)
{
	int i=0,j=0,k=0,p=0,q=0,g=0,n=0;
	vector<vector<vector<int>>> temporay_vector;
	temporay_vector.resize(new_vector_size);
	for (i=0;i!=new_vector_size;i++)
	{
		temporay_vector[i].resize(new_vector_size);
		for (j=0;j!=new_vector_size;j++)
		{
			temporay_vector[i][j].resize(new_vector_size);
			for (k=0;k!=new_vector_size;k++)
			{
				temporay_vector[i][j][k]=2;
			}
		}
	}

	///�жϽ��ϲ�������Ӧ��ǰ���������ֵ�෴���������Ϊ�����㣬�����ų��ϲ���Ϊȫ���ȫ1�����
	int sum_xy=0;
	int sum_xz=0;
	int sum_yz=0;
	int sum_3D=0;
	for (i=0;i!=new_vector_size;i++)
	{
		for (j=0;j!=new_vector_size;j++)
		{
			for (k=0;k!=new_vector_size;k++)
			{

				///////////////////////////////////////////////////////////////////
				if (i%2==1 && j%2==1 && k%2==1 && (Pre_process_vector[i][j][k]==2/*||Pre_process_vector[i][j][k]==300*/) ) //���Ԥ��������������ע3*3ȫΪ�ڻ�ȫ�׵�����
				{                                                                        //190212
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][j][(k+p+new_vector_size)%new_vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+p+new_vector_size)%new_vector_size][(k+p+new_vector_size)%new_vector_size];
						}
					}
					if (reconstruct_vector[i][j][k]==1 && sum_xy!=9 && sum_xz!=9 && sum_yz!=9)
					{
						temporay_vector[i][j][k]=1;
					}
					else if (reconstruct_vector[i][j][k]==0 && sum_xy!=0 && sum_xz!=0 && sum_yz!=0)
					{
						temporay_vector[i][j][k]=0;
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==1 && j%2==1 && k%2==0 && (Pre_process_vector[i][j][k]==2||Pre_process_vector[i][j][k]==300) )
				{                                                                            //190212
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][j][(k+p+new_vector_size)%new_vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+q+new_vector_size)%new_vector_size][(k+p+new_vector_size)%new_vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+g+new_vector_size)%new_vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						if (Pre_process_vector[i][j][k]!=300)     //190212
						{
							Pre_process_vector[i][j][k]=1;   //��ɹ̶���
						}
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (sum_xy!=9 && (sum_xz!=9 || sum_yz!=9) && reconstruct_vector[i][j][k]==1)
						{
							if (Pre_process_vector[i][j][k]!=300)    //190212
							{
								temporay_vector[i][j][k]=1;
							}
						}
						else if (sum_xy!=0 && (sum_xz!=0 || sum_yz!=0) && reconstruct_vector[i][j][k]==0)
						{
							if (Pre_process_vector[i][j][k]!=300)   //190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩   //�⿴��һ�����ã���Щ300���20/30��ѽ������ǰ��if�޶���preΪ2/300
							{
								temporay_vector[i][j][k]=0;
							}
						}
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==1 && j%2==0 && k%2==1 && (Pre_process_vector[i][j][k]==2||Pre_process_vector[i][j][k]==300) )
				{                                                                            //190212
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][j][(k+p+new_vector_size)%new_vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+q+new_vector_size)%new_vector_size][(k+p+new_vector_size)%new_vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+g+new_vector_size)%new_vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						if (Pre_process_vector[i][j][k]!=300)   //190212
						{
							Pre_process_vector[i][j][k]=1;
						}		
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (sum_xz!=9 && (sum_xy!=9 || sum_yz!=9) && reconstruct_vector[i][j][k]==1)
						{
							if (Pre_process_vector[i][j][k]!=300)   //190212
							{
								temporay_vector[i][j][k]=1;
							}
						}
						else if (sum_xz!=0 && (sum_xy!=0 || sum_yz!=0) && reconstruct_vector[i][j][k]==0)
						{
							if (Pre_process_vector[i][j][k]!=300)   //190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
							{
								temporay_vector[i][j][k]=0;
							}
						}
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==0 && j%2==1 && k%2==1 && (Pre_process_vector[i][j][k]==2||Pre_process_vector[i][j][k]==300) )
				{                                                                            //190212
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][j][(k+p+new_vector_size)%new_vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+q+new_vector_size)%new_vector_size][(k+p+new_vector_size)%new_vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+g+new_vector_size)%new_vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						if (Pre_process_vector[i][j][k]!=300)    //190212
						{
							Pre_process_vector[i][j][k]=1;
						}
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (sum_yz!=9 && (sum_xy!=9 || sum_xz!=9) && reconstruct_vector[i][j][k]==1)
						{
							if (Pre_process_vector[i][j][k]!=300)   //190212
							{
								temporay_vector[i][j][k]=1;
							}
						}
						else if (sum_yz!=0 && (sum_xy!=0 || sum_xz!=0) && reconstruct_vector[i][j][k]==0)
						{
							if (Pre_process_vector[i][j][k]!=300)   //190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
							{
								temporay_vector[i][j][k]=0;
							}
						}
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==0 && j%2==0 && k%2==1 && (Pre_process_vector[i][j][k]==2||Pre_process_vector[i][j][k]==300) )
				{                                                                            //190212
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][j][(k+p+new_vector_size)%new_vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+p+new_vector_size)%new_vector_size][(k+p+new_vector_size)%new_vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+g+new_vector_size)%new_vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						if (Pre_process_vector[i][j][k]!=300)    //190212
						{
							Pre_process_vector[i][j][k]=1;
						}
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (reconstruct_vector[i][j][k]==1 && (sum_xy!=9 || (sum_xz!=9 && sum_yz!=9)))
						{
							if (Pre_process_vector[i][j][k]!=300)    //190212
							{
								temporay_vector[i][j][k]=1;
							}
						}
						else if(reconstruct_vector[i][j][k]==0 && (sum_xy!=0 || (sum_xz!=0 && sum_yz!=0)))
						{
							if (Pre_process_vector[i][j][k]!=300)   //190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
							{
								temporay_vector[i][j][k]=0;
							}
						}
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==0 && j%2==1 && k%2==0 && (Pre_process_vector[i][j][k]==2||Pre_process_vector[i][j][k]==300) )
				{                                                                            //190212
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][j][(k+p+new_vector_size)%new_vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+p+new_vector_size)%new_vector_size][(k+p+new_vector_size)%new_vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+g+new_vector_size)%new_vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						if (Pre_process_vector[i][j][k]!=300)    //190212
						{
							Pre_process_vector[i][j][k]=1;
						}
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (reconstruct_vector[i][j][k]==1 && (sum_xz!=9 || (sum_xy!=9 && sum_yz!=9)))
						{
							if (Pre_process_vector[i][j][k]!=300)    //190212
							{
								temporay_vector[i][j][k]=1;
							}
						}
						else if(reconstruct_vector[i][j][k]==0 && (sum_xz!=0 || (sum_xy!=0 && sum_yz!=0)))
						{
							if (Pre_process_vector[i][j][k]!=300)   //190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
							{
								temporay_vector[i][j][k]=0;
							}
						}
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==1 && j%2==0 && k%2==0 && (Pre_process_vector[i][j][k]==2||Pre_process_vector[i][j][k]==300) )
				{                                                                            //190212
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][j][(k+p+new_vector_size)%new_vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+p+new_vector_size)%new_vector_size][(k+p+new_vector_size)%new_vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+g+new_vector_size)%new_vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						if (Pre_process_vector[i][j][k]!=300)    //190212
						{
							Pre_process_vector[i][j][k]=1;
						}
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (reconstruct_vector[i][j][k]==1 && (sum_yz!=9 || (sum_xy!=9 && sum_xz!=9)))
						{
							if (Pre_process_vector[i][j][k]!=300)   //190212
							{
								temporay_vector[i][j][k]=1;
							}
						}
						else if (reconstruct_vector[i][j][k]==0 && (sum_yz!=0 || (sum_xy!=0 && sum_xz!=0)))
						{
							if (Pre_process_vector[i][j][k]!=300)   //190212����׿׵�λ�ò���������ֻ��ΪС�׵ı����㣩
							{
								temporay_vector[i][j][k]=0;
							}
						}
					}
				}
			}
		}
	}
	//temporay_vector���ѡ�񽻻��㣨190212����׿׵��λ�ò��ܲ���������
	//Sign_exchange_vector��Ž����ɹ�������
	//Pre_process_vector����Ѿ�ȷ���ĵ㣬���ٲ��뽻��

	//////////////////////////////////////////////////////////ѡȡ������
	white_point_number=0;
	black_point_number=0;
	bool judgement=false; 

	for (i=0;i!=new_vector_size;i++)
	{
		for (j=0;j!=new_vector_size;j++)
		{
			for (k=0;k!=new_vector_size;k++)
			{
				if (temporay_vector[i][j][k]==1)
				{
					white_point_x.push_back(i);
					white_point_y.push_back(j);
					white_point_z.push_back(k);
					++white_point_number;
				}
				else if (temporay_vector[i][j][k]==0)
				{
					black_point_x.push_back(i);
					black_point_y.push_back(j);
					black_point_z.push_back(k);
					++black_point_number;
				}
			}
		}
	}

	class_white_point_number=white_point_number;
	class_black_point_number=black_point_number;
	temporay_vector.clear();
}

//���������ԭ����
void CAnnealing::Random_position(int loop)
{
	int rand_data_1=0;
	int rand_data_2=0;
	int temperory=0;
	int rand_number_white=(1.8+0.3*(loop%2))*class_white_point_number;
	int rand_number_black=(1.8+0.2*(loop%4))*class_black_point_number;
	while(rand_number_white!=0)
	{
		rand_data_1=rand32()%class_white_point_number;
		rand_data_2=rand32()%class_white_point_number;
		if (rand_data_2!=rand_data_1)
		{
			temperory=white_point_x[rand_data_1];
			white_point_x[rand_data_1]=white_point_x[rand_data_2];
			white_point_x[rand_data_2]=temperory;
			temperory=white_point_y[rand_data_1];
			white_point_y[rand_data_1]=white_point_y[rand_data_2];
			white_point_y[rand_data_2]=temperory;
			temperory=white_point_z[rand_data_1];
			white_point_z[rand_data_1]=white_point_z[rand_data_2];
			white_point_z[rand_data_2]=temperory;
			rand_number_white--;
		}
	}
	while(rand_number_black!=0)
	{
		rand_data_1=rand32()%class_black_point_number;
		rand_data_2=rand32()%class_black_point_number;
		if (rand_data_2!=rand_data_1)
		{
			temperory=black_point_x[rand_data_1];
			black_point_x[rand_data_1]=black_point_x[rand_data_2];
			black_point_x[rand_data_2]=temperory;
			temperory=black_point_y[rand_data_1];
			black_point_y[rand_data_1]=black_point_y[rand_data_2];
			black_point_y[rand_data_2]=temperory;
			temperory=black_point_z[rand_data_1];
			black_point_z[rand_data_1]=black_point_z[rand_data_2];
			black_point_z[rand_data_2]=temperory;
			rand_number_black--;
		}
	}
}

//white/black_rand_site ��/��ɫ��������λ��
void CAnnealing::Exchange_two_point()
{
	white_rand_site=class_white_point_number-1;
	black_rand_site=class_black_point_number-1;
	//white_rand_site=rand32()%class_white_point_number;
	//black_rand_site=rand32()%class_black_point_number;
}

void CAnnealing::Get_exchange_site()
{
	white_site_x= white_point_x.begin()+white_rand_site;
	white_site_y= white_point_y.begin()+white_rand_site;
	white_site_z= white_point_z.begin()+white_rand_site;
	black_site_x= black_point_x.begin()+black_rand_site;
	black_site_y= black_point_y.begin()+black_rand_site;
	black_site_z= black_point_z.begin()+black_rand_site;
}

void CAnnealing::Recstruct_Multi_point_density_function_fast_p(int& Engerry_down,int& Engerry_change_white,int& Engerry_change_black)
{ 

	Engerry_change_white=0;
	Engerry_change_black=0;
	int i=0,j=0;
	int code_value=0;//ģʽ����ֵ
	int q=0,p=0;
	int k=Rec_template_size/2;
	int white_x=white_point_x[white_rand_site];
	int white_y=white_point_y[white_rand_site];
	int white_z=white_point_z[white_rand_site];
	int black_x=black_point_x[black_rand_site];
	int black_y=black_point_y[black_rand_site];
	int black_z=black_point_z[black_rand_site];
	pair<map<int,int>::iterator,bool> pair_code_value;

	//�������Ƚ�Exchange_code_value_white/black����ˣ�������ִ�������--����ʱ��Ϊʲô�������
	Exchange_code_value_white.clear();
	Exchange_code_value_black.clear();
	for (q=-k;q!=k+1;q++)      ////��ѭ��Ϊÿ�������������9��ģʽ
	{
		for (p=-k;p!=k+1;p++)
		{
			reconstruct_vector[white_x][white_y][white_z]=1;//����ǰ(190212����������temporay_vector���ѡ��Ľ�����ȷ����)
			reconstruct_vector[black_x][black_y][black_z]=0;

			////////////////////////////////////////////////////XYƽ�潻���������ģʽ

			code_value=0;
			for (i=-k;i!=k+1;i++) ////��ѭ��Ϊ����ÿ��ģʽ
			{
				for (j=-k;j!=k+1;j++)
				{
					code_value=code_value*2+reconstruct_vector[(i+white_x+q+new_vector_size)%new_vector_size][(j+white_y+p+new_vector_size)%new_vector_size][white_z];
				}
			}
			--Exchange_code_value_white[code_value];//��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ


			if (white_z!=black_z)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[(i+black_x+q+new_vector_size)%new_vector_size][(j+black_y+p+new_vector_size)%new_vector_size][black_z];
					}
				}
				--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
			}

			else if (abs((black_x+q+new_vector_size)%new_vector_size-white_x)>k || abs((black_y+p+new_vector_size)%new_vector_size-white_y)>k)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[(i+black_x+q+new_vector_size)%new_vector_size][(j+black_y+p+new_vector_size)%new_vector_size][black_z];
					}
				}
				--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
			}

			////////////////////////////////////////////////////XZƽ�潻���������ģʽ
			code_value=0;
			for (i=-k;i!=k+1;i++)
			{
				for (j=-k;j!=k+1;j++)
				{
					code_value=code_value*2+reconstruct_vector[(i+white_x+q+new_vector_size)%new_vector_size][white_y][(j+white_z+p+new_vector_size)%new_vector_size];
				}
			}
			--Exchange_code_value_white[code_value];//��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ

			if (white_y!=black_y)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[(i+black_x+q+new_vector_size)%new_vector_size][black_y][(j+black_z+p+new_vector_size)%new_vector_size];
					}
				}
				--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
			}

			else if (abs((black_x+q+new_vector_size)%new_vector_size-white_x)>k || abs((black_z+p+new_vector_size)%new_vector_size-white_z)>k)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[(i+black_x+q+new_vector_size)%new_vector_size][black_y][(j+black_z+p+new_vector_size)%new_vector_size];
					}
				}
				--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
			}

			//////////////////////////////////////////////////YZƽ�潻���������ģʽ
			code_value=0;
			for (i=-k;i!=k+1;i++)
			{
				for (j=-k;j!=k+1;j++)
				{
					code_value=code_value*2+reconstruct_vector[white_x][(i+white_y+q+new_vector_size)%new_vector_size][(j+white_z+p+new_vector_size)%new_vector_size];
				}
			}
			--Exchange_code_value_white[code_value];//��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ

			if (white_x!=black_x)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[black_x][(i+black_y+q+new_vector_size)%new_vector_size][(j+black_z+p+new_vector_size)%new_vector_size];
					}
				}
				--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
			}
			else if (abs((black_y+q+new_vector_size)%new_vector_size-white_y)>k || abs((black_z+p+new_vector_size)%new_vector_size-white_z)>k)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[black_x][(i+black_y+q+new_vector_size)%new_vector_size][(j+black_z+p+new_vector_size)%new_vector_size];
					}
				}
				--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			reconstruct_vector[white_x][white_y][white_z]=0;//������
			reconstruct_vector[black_x][black_y][black_z]=1;
			////////////////////////////////////////////////////XYƽ�潻���������ģʽ
			code_value=0;
			for (i=-k;i!=k+1;++i)
			{
				for (j=-k;j!=k+1;++j)
				{
					code_value=code_value*2+reconstruct_vector[(i+white_x+q+new_vector_size)%new_vector_size][(j+white_y+p+new_vector_size)%new_vector_size][white_z];
				}
			}
			pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
			if(!pair_code_value.second)           
			{
				++pair_code_value.first->second; 
			}


			if (white_z!=black_z)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[(i+black_x+q+new_vector_size)%new_vector_size][(j+black_y+p+new_vector_size)%new_vector_size][black_z];
					}
				}
				pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
				{
					++pair_code_value.first->second;
				}
			}
			else if (abs((black_x+q+new_vector_size)%new_vector_size-white_x)>k || abs((black_y+p+new_vector_size)%new_vector_size-white_y)>k)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[(i+black_x+q+new_vector_size)%new_vector_size][(j+black_y+p+new_vector_size)%new_vector_size][black_z];
					}
				}
				pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
				{
					++pair_code_value.first->second;
				}
			}

			////////////////////////////////////////////////////YZƽ�潻���������ģʽ
			code_value=0;
			for (i=-k;i!=k+1;++i)
			{
				for (j=-k;j!=k+1;++j)
				{
					code_value=code_value*2+reconstruct_vector[white_x][(j+white_y+p+new_vector_size)%new_vector_size][(i+white_z+q+new_vector_size)%new_vector_size];
				}
			}
			pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
			if(!pair_code_value.second)           
			{
				++pair_code_value.first->second; 
			}


			if (white_x!=black_x)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[black_x][(j+black_y+p+new_vector_size)%new_vector_size][(i+black_z+q+new_vector_size)%new_vector_size];
					}
				}
				pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
				{
					++pair_code_value.first->second;
				}
			}
			else if (abs((black_y+q+new_vector_size)%new_vector_size-white_y)>k || abs((black_z+p+new_vector_size)%new_vector_size-white_z)>k)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[black_x][(j+black_y+p+new_vector_size)%new_vector_size][(i+black_z+q+new_vector_size)%new_vector_size];
					}
				}
				pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
				{
					++pair_code_value.first->second;
				}
			}


			////////////////////////////////////////////////////XZƽ�潻���������ģʽ
			code_value=0;
			for (i=-k;i!=k+1;++i)
			{
				for (j=-k;j!=k+1;++j)
				{
					code_value=code_value*2+reconstruct_vector[(i+white_x+q+new_vector_size)%new_vector_size][white_y][(j+white_z+p+new_vector_size)%new_vector_size];
				}
			}
			pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
			if(!pair_code_value.second)           
			{
				++pair_code_value.first->second; 
			}


			if (white_y!=black_y)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[(i+black_x+q+new_vector_size)%new_vector_size][black_y][(j+black_z+p+new_vector_size)%new_vector_size];
					}
				}
				pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
				{
					++pair_code_value.first->second;
				}
			}
			else if (abs((black_x+q+new_vector_size)%new_vector_size-white_x)>k || abs((black_z+p+new_vector_size)%new_vector_size-white_z)>k)
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[(i+black_x+q+new_vector_size)%new_vector_size][black_y][(j+black_z+p+new_vector_size)%new_vector_size];
					}
				}
				pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
				{
					++pair_code_value.first->second;
				}
			}
		}
	}
	int Engerry_before_exchange_white=0;
	int Engerry_after_exchange_white=0;
	int Engerry_before_exchange_black=0;
	int Engerry_after_exchange_black=0;
	map <int,int>::iterator iter_Success = Exchange_code_value_white.begin();
	for(iter_Success;iter_Success!=Exchange_code_value_white.end();++iter_Success)
	{
		//190113
		Engerry_before_exchange_white=Engerry_before_exchange_white+abs(Reconstruct_code_value[iter_Success->first]-3*new_vector_size*TI_code_value[iter_Success->first]*(beishu*beishu));//����û�б仯��ģʽ����
		Reconstruct_code_value[iter_Success->first]=Reconstruct_code_value[iter_Success->first]+Exchange_code_value_white[iter_Success->first];//����ԭ����ģʽ
		Engerry_after_exchange_white=Engerry_after_exchange_white+abs(Reconstruct_code_value[iter_Success->first]-3*new_vector_size*TI_code_value[iter_Success->first]*(beishu*beishu));//����ģʽ��ֵ
	}
	Engerry_change_white=Engerry_after_exchange_white-Engerry_before_exchange_white;

	iter_Success = Exchange_code_value_black.begin();
	for(iter_Success;iter_Success!=Exchange_code_value_black.end();++iter_Success)
	{
		Engerry_before_exchange_black=Engerry_before_exchange_black+abs(Reconstruct_code_value[iter_Success->first]-3*new_vector_size*TI_code_value[iter_Success->first]*(beishu*beishu));//����û�б仯��ģʽ����
		Reconstruct_code_value[iter_Success->first]=Reconstruct_code_value[iter_Success->first]+Exchange_code_value_black[iter_Success->first];//����ԭ����ģʽ
		Engerry_after_exchange_black=Engerry_after_exchange_black+abs(Reconstruct_code_value[iter_Success->first]-3*new_vector_size*TI_code_value[iter_Success->first]*(beishu*beishu));//����ģʽ��ֵ
	}
	Engerry_change_black=Engerry_after_exchange_black-Engerry_before_exchange_black;

	Enerry_Reverse=Enerry_origial;//������ǰ��������棬�Ա��滻����
	Engerry_down=Enerry_origial+Engerry_change_white+Engerry_change_black;  //���ݽ������������
	Enerry_origial=Engerry_down;//��������������Ϊ��״̬
	reconstruct_vector[white_x][white_y][white_z]=0;//������
	reconstruct_vector[black_x][black_y][black_z]=1;
}

void CAnnealing::Delete_exchange_point()
{
	//white_point_x.erase(white_site_x);
	//white_point_y.erase(white_site_y);
	//white_point_z.erase(white_site_z);
	//black_point_x.erase(black_site_x);
	//black_point_y.erase(black_site_y);
	//black_point_z.erase(black_site_z);
	class_white_point_number--;
	class_black_point_number--;
}

void CAnnealing::Recover_Multi_point_density_function()
{  
	map <int,int>::iterator iter_F = Exchange_code_value_black.begin();
	for(iter_F;iter_F!=Exchange_code_value_black.end();++iter_F)
	{
		Reconstruct_code_value[iter_F->first]=Reconstruct_code_value[iter_F->first]-Exchange_code_value_black[iter_F->first];
	}
	iter_F = Exchange_code_value_white.begin();
	for(iter_F;iter_F!=Exchange_code_value_white.end();++iter_F)
	{
		Reconstruct_code_value[iter_F->first]=Reconstruct_code_value[iter_F->first]-Exchange_code_value_white[iter_F->first];
	}
	Enerry_origial=Enerry_Reverse;
}

void CAnnealing::Reverse_exchange_point()
{
	reconstruct_vector[white_point_x[white_rand_site]][white_point_y[white_rand_site]][white_point_z[white_rand_site]]=1;
	reconstruct_vector[black_point_x[black_rand_site]][black_point_y[black_rand_site]][black_point_z[black_rand_site]]=0;
}

void CAnnealing::Delete_unexchange_white_point()
{
	//white_point_x.erase(white_site_x);
	//white_point_y.erase(white_site_y);
	//white_point_z.erase(white_site_z);
	class_white_point_number--;
}

void CAnnealing::Delete_unexchange_black_point()
{
	//black_point_x.erase(black_site_x);
	//black_point_y.erase(black_site_y);
	//black_point_z.erase(black_site_z);
	class_black_point_number--;
}

void CAnnealing::Delete_unexchange_two_phase_point()
{
	//white_point_x.erase(white_site_x);
	//white_point_y.erase(white_site_y);
	//white_point_z.erase(white_site_z);
	//black_point_x.erase(black_site_x);
	//black_point_y.erase(black_site_y);
	//black_point_z.erase(black_site_z);
	class_white_point_number--;
	class_black_point_number--;
}

void CAnnealing::Delete_site_vector()
{
	white_point_x.clear();
	white_point_y.clear();
	white_point_z.clear();
	black_point_x.clear();
	black_point_y.clear();
	black_point_z.clear();
	class_white_point_number=0;
	class_black_point_number=0;
}

void CAnnealing::Set_reconstruct_to_inital()
{
	int i=0,j=0,k=0;
	inital_rand_data.resize(new_vector_size); //190113
	for (i=0;i!=new_vector_size;++i)
	{
		inital_rand_data[i].resize(new_vector_size);
		for (j=0;j!=new_vector_size;++j)
		{
			inital_rand_data[i][j].resize(new_vector_size);
		}
	}
	for (i=0;i!=new_vector_size;++i)
	{
		for (j=0;j!=new_vector_size;++j)
		{
			for (k=0;k!=new_vector_size;k++)
			{
				inital_rand_data[i][j][k]=reconstruct_vector[i][j][k];
			}
		}
	}
}

void CAnnealing::Putout_step_image()
{
	int i=0,j=0,k=0,q=0,white_point=0;
	for (q=0;q!=new_vector_size;q++)
	{
		IplImage* pImage=cvCreateImage(cvSize(new_vector_size,new_vector_size), IPL_DEPTH_8U, 1);//�洢ͼƬ	
		int step =pImage->widthStep;  //ÿ��ͼ��ÿ���ֽ���
		uchar *data=(uchar *)pImage->imageData;
		for (i=0;i!=new_vector_size;++i)
		{
			for (j=0;j!=new_vector_size;++j)
			{
				k=reconstruct_vector[q][i][j];
				if (k==1)
				{
					pImage->imageData[i*step+j]=255;	
					//white_point++;
				}
				else
				{
					pImage->imageData[i*step+j]=0;
				}
			}
		}
		//cout<<"�ؽ����ɫ����:"<<white_point<<endl;
		char name[100];
		if (q<10)
		{
			sprintf(name,"number 00%d.bmp",q);
		}
		else if (q<100)
		{
			sprintf(name,"number 0%d.bmp",q);
		}
		else
		{
			sprintf(name,"number %d.bmp",q);
		}
		cvSaveImage(name, pImage);//����ͼƬ����ʽΪ���ֱ�ţ�����1.bmp��
		cvReleaseImage(&pImage);
	}
}

//190213�����������׵���ά��϶�ṹ
void CAnnealing::Set_reconstruct_to_final()
{
	int i=0,j=0,k=0;
	if (final_imgsize!=new_vector_size)
	{
		cout<<"�ؽ��ߴ����"<<endl;
		return;
	}

	inital_rand_data.resize(final_imgsize); //190113
	for (i=0;i!=final_imgsize;++i)
	{
		inital_rand_data[i].resize(final_imgsize);
		for (j=0;j!=final_imgsize;++j)
		{
			inital_rand_data[i][j].resize(final_imgsize);
		}
	}
	for (i=0;i!=final_imgsize;++i)
	{
		for (j=0;j!=final_imgsize;++j)
		{
			for (k=0;k!=final_imgsize;k++)
			{
				if (big_3Dimg[i][j][k]!=0||reconstruct_vector[i][j][k]!=0)
				{
					inital_rand_data[i][j][k]=255;
				}
				else
				{
					inital_rand_data[i][j][k]=0;
				}	
			}
		}
	}

	Putout_final_image();
}

void CAnnealing::Putout_final_image()
{
	AfxSetResourceHandle(GetModuleHandle(NULL));       ////�ڿ���̨������ʹ��CFileDialog�����Ҫ���
	AfxMessageBox(_T("��ѡ�񱣴��ںϽ�����ļ��У�"));

	string savpth;
	for (int i=0;i<final_imgsize;i++)
	{
		if (i==0)
		{
			CFileDialog dlgSave(FALSE,"*.bmp","0000",OFN_CREATEPROMPT|OFN_PATHMUSTEXIST,"BMP Files(*.bmp)|*.bmp|All Files(*.*)|*.*||");
			if (dlgSave.DoModal()==IDOK)
			{
				CString cSavPth=dlgSave.GetFolderPath();
				savpth=cSavPth.GetBuffer(0);
				CString cimg_sav=dlgSave.GetPathName();
				string img_sav=cimg_sav.GetBuffer(0);

				Mat cur_img=Mat::zeros(final_imgsize,final_imgsize,CV_8UC1);
				for (int j=0;j<final_imgsize;j++)
				{
					for (int k=0;k<final_imgsize;k++)
					{
						cur_img.at<uchar>(j,k)=inital_rand_data[i][j][k];
					}
				}
				imwrite(img_sav,cur_img);
			}
		}
		else
		{
			char file[MAX_PATH];
			string imgSavPth=savpth+"\\"+"%04d.bmp";
			const char* c_s=imgSavPth.c_str();
			sprintf(file,c_s,i);
			Mat cur_img=Mat::zeros(final_imgsize,final_imgsize,CV_8UC1);
			for (int j=0;j<final_imgsize;j++)
			{
				for (int k=0;k<final_imgsize;k++)
				{
					cur_img.at<uchar>(j,k)=inital_rand_data[i][j][k];
				}
			}
			imwrite(file,cur_img);

		}
	}
	cout<<"�ؽ��������ɹ���"<<endl;
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void CAnnealing::TI_Multi_point_density_function()
{
	TI_code_value.clear();//����һ����洢�Ķ���ܶȺ������
	int i=0,j=0;
	int reduce_size=inital_height/vector_size;//��С����
	int k=Rec_template_size/2;
	vector<vector<int>> Temporary_vector;
	Temporary_vector.resize(vector_size);
	for(i=0;i!=vector_size;++i)
	{
		Temporary_vector[i].resize(vector_size);
	}

	//����Ӧ���ԭʼͼ��ŵ��м��������
	for (i=0;i!=vector_size;i++)
	{
		for (j=0;j!=vector_size;j++)
		{
			if (Inital_image_vector[i*reduce_size][j*reduce_size]==1)
			{
				Temporary_vector[i][j]=1;//Ϊ�˺�����뷽�㽫255ֵ��Ϊ1
			}
			else
			{
				Temporary_vector[i][j]=0;
			}
		}
	}

	//�����ʱ��Сͼ��Ķ���ܶȺ���
	int code_value=0;//ģʽ����ֵ
	int q=0,p=0;
	for (i=0;i!=vector_size;++i)
	{
		for (j=0;j!=vector_size;++j)
		{
			code_value=0;
			for (p=-k;p!=k+1;p++)
			{
				for (q=-k;q!=k+1;q++)
				{
					if (i+p>=0 && i+p<vector_size && j+q>=0 && j+q<vector_size)
					{
						code_value=code_value*2+Temporary_vector[i+p][j+q];
					}
				}
			}
			pair<map<int,int>::iterator,bool> pair_code_value=TI_code_value.insert(make_pair(code_value,1));
			if(!pair_code_value.second)           
			{
				++pair_code_value.first->second; 
			}
		}
	}
}

void CAnnealing::Recstruct_Multi_point_density_function()
{
	Reconstruct_code_value.clear();//����һ����洢�Ķ���ܶȺ������  
	int i=0,j=0,k=0,q=0,p=0,r=0;
	int s=Rec_template_size/2;
	pair<map<int,int>::iterator,bool> pair_code_value;//�����ʱ�ؽ�ͼ��Ķ���ܶȺ���
	int code_value=0;//ģʽ����ֵ
	for (k=0;k!=vector_size;++k)
	{
		for (j=0;j!=vector_size;++j)
		{
			for (i=0;i!=vector_size;++i)
			{
				code_value=0;
				for (p=-s;p!=s+1;p++)
				{
					for (q=-s;q!=s+1;q++)
					{
						if (i+p>=0 && i+p<vector_size && j+q>=0 && j+q<vector_size)
						{
							code_value=code_value*2+reconstruct_vector[k][i+p][j+q];
						}
					}
				}
				pair_code_value=Reconstruct_code_value.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           
				{
					++pair_code_value.first->second; 
				}
				code_value=0;
				for (p=-s;p!=s+1;p++)
				{
					for (q=-s;q!=s+1;q++)
					{
						if (i+p>=0 && i+p<vector_size && j+q>=0 && j+q<vector_size)
						{
							code_value=code_value*2+reconstruct_vector[i+p][k][j+q];
						}
					}
				}
				pair_code_value=Reconstruct_code_value.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           
				{
					++pair_code_value.first->second; 
				}
				code_value=0;
				for (p=-s;p!=s+1;p++)
				{
					for (q=-s;q!=s+1;q++)
					{
						if (i+p>=0 && i+p<vector_size && j+q>=0 && j+q<vector_size)
						{
							code_value=code_value*2+reconstruct_vector[i+p][j+q][k];
						}
					}
				}
				pair_code_value=Reconstruct_code_value.insert(make_pair(code_value,1));
				if(!pair_code_value.second)           
				{
					++pair_code_value.first->second; 
				}
			}
		}
	}	
}

void CAnnealing::Recstruct_Multi_point_density_function_fast(int& Engerry_down,int& Engerry_change_white,int& Engerry_change_black)
{ 

	Engerry_change_white=0;
	Engerry_change_black=0;
	int i=0,j=0;
	int code_value=0;//ģʽ����ֵ
	int q=0,p=0;
	int k=Rec_template_size/2;
	int white_x=white_point_x[white_rand_site];
	int white_y=white_point_y[white_rand_site];
	int white_z=white_point_z[white_rand_site];
	int black_x=black_point_x[black_rand_site];
	int black_y=black_point_y[black_rand_site];
	int black_z=black_point_z[black_rand_site];
	pair<map<int,int>::iterator,bool> pair_code_value;
	Exchange_code_value_white.clear();
	Exchange_code_value_black.clear();
	for (q=-k;q!=k+1;q++)      
	{
		for (p=-k;p!=k+1;p++)
		{
			reconstruct_vector[white_x][white_y][white_z]=1;//����ǰ
			reconstruct_vector[black_x][black_y][black_z]=0;
			////////////////////////////////////////////////////XYƽ�潻���������ģʽ
			if ((white_x+q>=k) && (white_x+q<vector_size-k) && (white_y+p>=k) && (white_y+p<vector_size-k))
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[i+white_x+q][j+white_y+p][white_z];
					}
				}
				--Exchange_code_value_white[code_value];//��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ
			}
			if ((black_x+q>=k) && (q+black_x<vector_size-k) && (black_y+p>=k) && (black_y+p<vector_size-k))
			{
				if (white_z!=black_z)
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[i+black_x+q][j+black_y+p][black_z];
						}
					}
					--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
				}
				else if (((black_x+q)>(white_x+k)) || ((black_x+q)<(white_x-k)) || ((black_y+p)>(white_y+k)) || ((black_y+p)<(white_y-k)))
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[i+black_x+q][j+black_y+p][black_z];
						}
					}
					--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
				}
			}
			////////////////////////////////////////////////////XZƽ�潻���������ģʽ
			if ((white_x+q>=k) && (white_x+q<vector_size-k) && (white_z+p>=k) && (white_z+p<vector_size-k))
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[i+white_x+q][white_y][j+white_z+p];
					}
				}
				--Exchange_code_value_white[code_value];//��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ
			}
			if ((black_x+q>=k) && (q+black_x<vector_size-k) && (black_z+p>=k) &&(black_z+p<vector_size-k))
			{
				if (white_y!=black_y)
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[i+black_x+q][black_y][j+black_z+p];
						}
					}
					--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
				}
				else if (((black_x+q)>(white_x+k)) || ((black_x+q)<(white_x-k)) || ((black_z+p)>(white_z+k)) || ((black_z+p)<(white_z-k)))
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[i+black_x+q][black_y][j+black_z+p];
						}
					}
					--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
				}
			}
			//////////////////////////////////////////////////YZƽ�潻���������ģʽ
			if ((white_y+q>=k) && (white_y+q<vector_size-k) && (white_z+p>=k) && (white_z+p<vector_size-k))
			{
				code_value=0;
				for (i=-k;i!=k+1;i++)
				{
					for (j=-k;j!=k+1;j++)
					{
						code_value=code_value*2+reconstruct_vector[white_x][i+white_y+q][j+white_z+p];
					}
				}
				--Exchange_code_value_white[code_value];//��ȥ����ǰ�ɰ�ɫ������ȷ����ģʽ
			}
			if ((black_y+q>=k) && (q+black_y<vector_size-k) && (black_z+p>=k) &&(black_z+p<vector_size-k))
			{
				if (white_x!=black_x)
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[black_x][i+black_y+q][j+black_z+p];
						}
					}
					--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
				}
				else if (((black_y+q)>(white_y+k)) || ((black_y+q)<(white_y-k)) || ((black_z+p)>(white_z+k)) || ((black_z+p)<(white_z-k)))
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[black_x][i+black_y+q][j+black_z+p];
						}
					}
					--Exchange_code_value_black[code_value];//��ȥ����ǰ�ɺ�ɫ������ȷ����ģʽ
				}
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			reconstruct_vector[white_x][white_y][white_z]=0;//������
			reconstruct_vector[black_x][black_y][black_z]=1;
			////////////////////////////////////////////////////XYƽ�潻���������ģʽ
			if ((white_x+q>=k) && (white_x+q<vector_size-k) && (white_y+p>=k) && (white_y+p<vector_size-k))
			{
				code_value=0;
				for (i=-k;i!=k+1;++i)
				{
					for (j=-k;j!=k+1;++j)
					{
						code_value=code_value*2+reconstruct_vector[i+white_x+q][j+white_y+p][white_z];
					}
				}
				pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
				if(!pair_code_value.second)           
				{
					++pair_code_value.first->second; 
				}
			}

			if ((black_x+q>=k) && (q+black_x<vector_size-k) && (black_y+p>=k) &&(black_y+p<vector_size-k))
			{
				if (white_z!=black_z)
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[i+black_x+q][j+black_y+p][black_z];
						}
					}
					pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
					if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
					{
						++pair_code_value.first->second;
					}
				}
				else if (((black_x+q)>(white_x+k)) || ((black_x+q)<(white_x-k)) || ((black_y+p)>(white_y+k)) || ((black_y+p)<(white_y-k)))
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[i+black_x+q][j+black_y+p][black_z];
						}
					}
					pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
					if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
					{
						++pair_code_value.first->second;
					}
				}
			}

			////////////////////////////////////////////////////YZƽ�潻���������ģʽ
			if ((white_z+q>=k) && (white_z+q<vector_size-k) && (white_y+p>=k) && (white_y+p<vector_size-k))
			{
				code_value=0;
				for (i=-k;i!=k+1;++i)
				{
					for (j=-k;j!=k+1;++j)
					{
						code_value=code_value*2+reconstruct_vector[white_x][j+white_y+p][i+white_z+q];
					}
				}
				pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
				if(!pair_code_value.second)           
				{
					++pair_code_value.first->second; 
				}
			}

			if ((black_z+q>=k) && (q+black_z<vector_size-k) && (black_y+p>=k) &&(black_y+p<vector_size-k))
			{
				if (white_x!=black_x)
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[black_x][j+black_y+p][i+black_z+q];
						}
					}
					pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
					if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
					{
						++pair_code_value.first->second;
					}
				}
				else if (((black_z+q)>(white_z+k)) || ((black_z+q)<(white_z-k)) || ((black_y+p)>(white_y+k)) || ((black_y+p)<(white_y-k)))
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[black_x][j+black_y+p][i+black_z+q];
						}
					}
					pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
					if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
					{
						++pair_code_value.first->second;
					}
				}
			}

			////////////////////////////////////////////////////XZƽ�潻���������ģʽ
			if ((white_x+q>=k) && (white_x+q<vector_size-k) && (white_z+p>=k) && (white_z+p<vector_size-k))
			{
				code_value=0;
				for (i=-k;i!=k+1;++i)
				{
					for (j=-k;j!=k+1;++j)
					{
						code_value=code_value*2+reconstruct_vector[i+white_x+q][white_y][j+white_z+p];
					}
				}
				pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//���Ͻ���ǰ�ɰ�ɫ������ȷ����ģʽ
				if(!pair_code_value.second)           
				{
					++pair_code_value.first->second; 
				}
			}

			if ((black_x+q>=k) && (q+black_x<vector_size-k) && (black_z+p>=k) &&(black_z+p<vector_size-k))
			{
				if (white_y!=black_y)
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[i+black_x+q][black_y][j+black_z+p];
						}
					}
					pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
					if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
					{
						++pair_code_value.first->second;
					}
				}
				else if (((black_x+q)>(white_x+k)) || ((black_x+q)<(white_x-k)) || ((black_z+p)>(white_z+k)) || ((black_z+p)<(white_z-k)))
				{
					code_value=0;
					for (i=-k;i!=k+1;i++)
					{
						for (j=-k;j!=k+1;j++)
						{
							code_value=code_value*2+reconstruct_vector[i+black_x+q][black_y][j+black_z+p];
						}
					}
					pair_code_value=Exchange_code_value_black.insert(make_pair(code_value,1));
					if(!pair_code_value.second)           //���Ͻ���ǰ�ɺ�ɫ������ȷ����ģʽ
					{
						++pair_code_value.first->second;
					}
				}
			}
		}
	}
	int Engerry_before_exchange_white=0;
	int Engerry_after_exchange_white=0;
	int Engerry_before_exchange_black=0;
	int Engerry_after_exchange_black=0;
	map <int,int>::iterator iter_Success = Exchange_code_value_white.begin();
	for(iter_Success;iter_Success!=Exchange_code_value_white.end();++iter_Success)
	{
		Engerry_before_exchange_white=Engerry_before_exchange_white+abs(Reconstruct_code_value[iter_Success->first]-3*vector_size*TI_code_value[iter_Success->first]);//����û�б仯��ģʽ����
		Reconstruct_code_value[iter_Success->first]=Reconstruct_code_value[iter_Success->first]+Exchange_code_value_white[iter_Success->first];//����ԭ����ģʽ
		Engerry_after_exchange_white=Engerry_after_exchange_white+abs(Reconstruct_code_value[iter_Success->first]-3*vector_size*TI_code_value[iter_Success->first]);//����ģʽ��ֵ
	}
	Engerry_change_white=Engerry_after_exchange_white-Engerry_before_exchange_white;

	iter_Success = Exchange_code_value_black.begin();
	for(iter_Success;iter_Success!=Exchange_code_value_black.end();++iter_Success)
	{
		Engerry_before_exchange_black=Engerry_before_exchange_black+abs(Reconstruct_code_value[iter_Success->first]-3*vector_size*TI_code_value[iter_Success->first]);//����û�б仯��ģʽ����
		Reconstruct_code_value[iter_Success->first]=Reconstruct_code_value[iter_Success->first]+Exchange_code_value_black[iter_Success->first];//����ԭ����ģʽ
		Engerry_after_exchange_black=Engerry_after_exchange_black+abs(Reconstruct_code_value[iter_Success->first]-3*vector_size*TI_code_value[iter_Success->first]);//����ģʽ��ֵ
	}
	Engerry_change_black=Engerry_after_exchange_black-Engerry_before_exchange_black;

	Enerry_Reverse=Enerry_origial;//������ǰ��������棬�Ա��滻����
	Engerry_down=Enerry_origial+Engerry_change_white+Engerry_change_black;  //���ݽ������������
	Enerry_origial=Engerry_down;//��������������Ϊ��״̬
	reconstruct_vector[white_x][white_y][white_z]=0;//������
	reconstruct_vector[black_x][black_y][black_z]=1;
}

void CAnnealing::Select_final_exchange_point_uncondition_fast(int& white_point_number,int& black_point_number)
{
	int i=0,j=0,k=0,p=0,q=0,g=0,n=0;
	int sum_white=0,sum_black=0;
	vector<vector<vector<int>>> temporay_vector;
	temporay_vector.resize(vector_size);
	for (i=0;i!=vector_size;i++)
	{
		temporay_vector[i].resize(vector_size);
		for (j=0;j!=vector_size;j++)
		{
			temporay_vector[i][j].resize(vector_size);
			for (k=0;k!=vector_size;k++)
			{
				temporay_vector[i][j][k]=2;
			}
		}
	}

	///�жϽ��ϲ�������Ӧ��ǰ���������ֵ�෴���������Ϊ�����㣬�����ų��ϲ���Ϊȫ���ȫ1�����
	for (i=0;i!=vector_size;i++)
	{
		for (j=0;j!=vector_size;j++)
		{
			for (k=0;k!=vector_size;k++)
			{
				if (i%2==0 && j%2==0 && k%2==0)
				{
					if (reconstruct_vector[i][j][k]==1)
					{
						for (q=-1;q!=2;q++)
						{
							for (p=-1;p!=2;p++)
							{
								for (g=-1;g!=2;g++)
								{
									if (reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size]==0)
									{
										temporay_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size]=0;
									}
								}
							}
						}
					}
					else
					{
						for (q=-1;q!=2;q++)
						{
							for (p=-1;p!=2;p++)
							{
								for (g=-1;g!=2;g++)
								{
									if (reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size]==1)
									{
										temporay_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size]=1;
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
	white_point_number=0;
	black_point_number=0;
	for (i=0;i!=vector_size;i++)
	{
		for (j=0;j!=vector_size;j++)
		{
			for (k=0;k!=vector_size;k++)
			{
				if (temporay_vector[i][j][k]==1 )
				{
					white_point_x.push_back(i);
					white_point_y.push_back(j);
					white_point_z.push_back(k);
					++white_point_number;
				}
				else if (temporay_vector[i][j][k]==0 )
				{
					black_point_x.push_back(i);
					black_point_y.push_back(j);
					black_point_z.push_back(k);
					++black_point_number;
				}
			}
		}
	}
	class_white_point_number=white_point_number;
	class_black_point_number=black_point_number;
	temporay_vector.clear();
}

void CAnnealing::Set_inital_to_reconstruct_uncondition_3()
{
	int i=0,j=0,k=0,p=0,q=0;
	int sum=0;
	int up_grid_white_number=0;
	int current_grid_white_number=0;
	int inital_white_number=0;
	int add_white_number=0;
	int up_white_number=0;
	int distance_point=inital_width/vector_size;
	/////////////////////////////////////////////////���о���������ֵ�����ϲ�ͼ�񴫵ݸ��ؽ�������
	reconstruct_vector.resize(vector_size);
	for (i=0;i!=vector_size;++i)
	{
		reconstruct_vector[i].resize(vector_size);
		for (j=0;j!=vector_size;++j)
		{  
			reconstruct_vector[i][j].resize(vector_size);
			for (k=0;k!=vector_size;k++)
			{
				if (i%2==0 && j%2==0 && k%2==0)
				{
					reconstruct_vector[i][j][k]=inital_rand_data[i/2][j/2][k/2];     /////���ϲ��ؽ��Ľ�����ݵ���Ӧ�����
					if (inital_rand_data[i/2][j/2][k/2]==1)
					{
						up_white_number++;
					}
				}
				else
				{
					reconstruct_vector[i][j][k]=2;                        /////����㸳��ֵ2
				}
			}
		}
	}

	for (i=0;i!=vector_size;++i)
	{
		for (j=0;j!=vector_size;++j)
		{
			if (Inital_image_vector[i*distance_point][j*distance_point]==1)/////ͳ�ƶ�Ӧ�����ɫ����
			{
				inital_white_number++;
			}
		}
	}
	add_white_number=vector_size*inital_white_number-up_white_number;
	int point_x=0;
	int point_y=0;
	int point_z=0;
	/////////////////////////////////////////////////////////���ؽ���������λ�ð��տ�϶�������ֵ��
	while (add_white_number!=0)
	{
		point_x=rand32()%vector_size;
		point_y=rand32()%vector_size;
		point_z=rand32()%vector_size;
		if ((point_y%2==1 || point_x%2==1 || point_z%2==1) && reconstruct_vector[point_x][point_y][point_z]==2)
		{
			reconstruct_vector[point_x][point_y][point_z]=1;
			add_white_number--;
		}
	}

	for (i=0;i!=vector_size;i++)
	{
		for (j=0;j!=vector_size;j++)
		{
			for (k=0;k!=vector_size;k++)
			{
				if ((i%2==1 || j%2==1 || k%2==1) && reconstruct_vector[i][j][k]==2)
				{
					reconstruct_vector[i][j][k]=0;
				}
			}

		}
	}
}

//
//void CAnnealing::Select_final_exchange_point_uncondition_test(int& white_point_number,int& black_point_number)
//{
//	int i=0,j=0,k=0,p=0,q=0,g=0,n=0;
//	int sum_xy=0;
//	int sum_xz=0;
//	int sum_yz=0;
//	vector<vector<vector<int>>> temporay_vector;
//	temporay_vector.resize(vector_size);
//	for (i=0;i!=vector_size;i++)
//	{
//		temporay_vector[i].resize(vector_size);
//		for (j=0;j!=vector_size;j++)
//		{
//			temporay_vector[i][j].resize(vector_size);
//			for (k=0;k!=vector_size;k++)
//			{
//				temporay_vector[i][j][k]=2;
//			}
//		}
//	}
//
//	///����������ѡ�㷨�����ų��ϲ���Ϊȫ���ȫ1�����
//	for (i=0;i!=vector_size;i++)
//	{
//		for (j=0;j!=vector_size;j++)
//		{
//			for (k=0;k!=vector_size;k++)
//			{
//				if (i%2==0 && j%2==0 && k%2==0)
//				{
//					if (reconstruct_vector[i][j][k]==1)
//					{
//						for (q=-1;q!=2;q++)
//						{
//							for (p=-1;p!=2;p++)
//							{
//								for (g=-1;g!=2;g++)
//								{
//									if (reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size]==0 && Pre_process_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size]==2 )
//									{
//										temporay_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size]=0;
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
//								for (g=-1;g!=2;g++)
//								{
//									if (reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size]==1 && Pre_process_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size]==2)
//									{
//										temporay_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size]=1;
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
//	for (i=0;i!=vector_size;i++)
//	{
//		for (j=0;j!=vector_size;j++)
//		{
//			for (k=0;k!=vector_size;k++)
//			{
//				sum_xy=0;
//				sum_xz=0;
//				sum_yz=0;
//				for (p=-1;p!=2;p++)
//				{
//					for (q=-1;q!=2;q++)
//					{
//						sum_xy=sum_xy+reconstruct_vector[(i+p+vector_size)%vector_size][(j+q+vector_size)%vector_size][k];
//					}
//				}
//				for (p=-1;p!=2;p++)
//				{
//					for (q=-1;q!=2;q++)
//					{
//						sum_xz=sum_xz+reconstruct_vector[(i+p+vector_size)%vector_size][j][(k+q+vector_size)%vector_size];
//					}
//				}
//				for (p=-1;p!=2;p++)
//				{
//					for (q=-1;q!=2;q++)
//					{
//						sum_yz=sum_yz+reconstruct_vector[i][(j+p+vector_size)%vector_size][(k+q+vector_size)%vector_size];
//					}
//				}
//				if ((sum_xy==8 || sum_xz==8 || sum_yz==8 ) && reconstruct_vector[i][j][k]==0 && Pre_process_vector[i][j][k]==2)
//				{
//					temporay_vector[i][j][k]=0;
//				}
//			}
//		}
//	}
//	//////////////////////////////////////////////////////////ѡȡ������
//	white_point_number=0;
//	black_point_number=0;
//	for (i=0;i!=vector_size;i++)
//	{
//		for (j=0;j!=vector_size;j++)
//		{
//			for (k=0;k!=vector_size;k++)
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



void CAnnealing::Select_final_exchange_point_uncondition_test(int& white_point_number,int& black_point_number)
{
	int i=0,j=0,k=0,p=0,q=0,g=0,n=0;
	vector<vector<vector<int>>> temporay_vector;
	temporay_vector.resize(vector_size);
	for (i=0;i!=vector_size;i++)
	{
		temporay_vector[i].resize(vector_size);
		for (j=0;j!=vector_size;j++)
		{
			temporay_vector[i][j].resize(vector_size);
			for (k=0;k!=vector_size;k++)
			{
				temporay_vector[i][j][k]=2;
			}
		}
	}

	///�жϽ��ϲ�������Ӧ��ǰ���������ֵ�෴���������Ϊ�����㣬�����ų��ϲ���Ϊȫ���ȫ1�����
	int sum_xy=0;
	int sum_xz=0;
	int sum_yz=0;
	int sum_3D=0;
	for (i=0;i!=vector_size;i++)
	{
		for (j=0;j!=vector_size;j++)
		{
			for (k=0;k!=vector_size;k++)
			{

				/////////////////////////////////////////////////////////////////////
				if (i%2==1 && j%2==1 && k%2==1 && Pre_process_vector[i][j][k]==2)
				{
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+vector_size)%vector_size][j][(k+p+vector_size)%vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+p+vector_size)%vector_size][(k+p+vector_size)%vector_size];
						}
					}
					if (reconstruct_vector[i][j][k]==1 && sum_xy!=9 && sum_xz!=9 && sum_yz!=9)
					{
						temporay_vector[i][j][k]=1;
					}
					else if (reconstruct_vector[i][j][k]==0 && sum_xy!=0 && sum_xz!=0 && sum_yz!=0)
					{
						temporay_vector[i][j][k]=0;
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==1 && j%2==1 && k%2==0 && Pre_process_vector[i][j][k]==2)
				{
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+vector_size)%vector_size][j][(k+p+vector_size)%vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+q+vector_size)%vector_size][(k+p+vector_size)%vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						Pre_process_vector[i][j][k]=1;
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (sum_xy!=9 && (sum_xz!=9 || sum_yz!=9) && reconstruct_vector[i][j][k]==1)
						{
							temporay_vector[i][j][k]=1;
						}
						else if (sum_xy!=0 && (sum_xz!=0 || sum_yz!=0) && reconstruct_vector[i][j][k]==0)
						{
							temporay_vector[i][j][k]=0;
						}
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==1 && j%2==0 && k%2==1 && Pre_process_vector[i][j][k]==2)
				{
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+vector_size)%vector_size][j][(k+p+vector_size)%vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+q+vector_size)%vector_size][(k+p+vector_size)%vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						Pre_process_vector[i][j][k]=1;
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (sum_xz!=9 && (sum_xy!=9 || sum_yz!=9) && reconstruct_vector[i][j][k]==1)
						{
							temporay_vector[i][j][k]=1;
						}
						else if (sum_xz!=0 && (sum_xy!=0 || sum_yz!=0) && reconstruct_vector[i][j][k]==0)
						{
							temporay_vector[i][j][k]=0;
						}
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==0 && j%2==1 && k%2==1 && Pre_process_vector[i][j][k]==2)
				{
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+vector_size)%vector_size][j][(k+p+vector_size)%vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+q+vector_size)%vector_size][(k+p+vector_size)%vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						Pre_process_vector[i][j][k]=1;
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (sum_yz!=9 && (sum_xy!=9 || sum_xz!=9) && reconstruct_vector[i][j][k]==1)
						{
							temporay_vector[i][j][k]=1;
						}
						else if (sum_yz!=0 && (sum_xy!=0 || sum_xz!=0) && reconstruct_vector[i][j][k]==0)
						{
							temporay_vector[i][j][k]=0;
						}
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==0 && j%2==0 && k%2==1 && Pre_process_vector[i][j][k]==2)
				{
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+vector_size)%vector_size][j][(k+p+vector_size)%vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+p+vector_size)%vector_size][(k+p+vector_size)%vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						Pre_process_vector[i][j][k]=1;
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (reconstruct_vector[i][j][k]==1 && (sum_xy!=9 || (sum_xz!=9 && sum_yz!=9)))
						{
							temporay_vector[i][j][k]=1;
						}
						else if(reconstruct_vector[i][j][k]==0 && (sum_xy!=0 || (sum_xz!=0 && sum_yz!=0)))
						{
							temporay_vector[i][j][k]=0;
						}
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==0 && j%2==1 && k%2==0 && Pre_process_vector[i][j][k]==2)
				{
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+vector_size)%vector_size][j][(k+p+vector_size)%vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+p+vector_size)%vector_size][(k+p+vector_size)%vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						Pre_process_vector[i][j][k]=1;
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (reconstruct_vector[i][j][k]==1 && (sum_xz!=9 || (sum_xy!=9 && sum_yz!=9)))
						{
							temporay_vector[i][j][k]=1;
						}
						else if(reconstruct_vector[i][j][k]==0 && (sum_xz!=0 || (sum_xy!=0 && sum_yz!=0)))
						{
							temporay_vector[i][j][k]=0;
						}
					}
				}

				/////////////////////////////////////////////////////////////////////
				else if (i%2==1 && j%2==0 && k%2==0 && Pre_process_vector[i][j][k]==2)
				{
					sum_3D=0;
					sum_xy=0;
					sum_xz=0;
					sum_yz=0;
					for (q=-1;q!=2;q++)
					{
						for (p=-1;p!=2;p++)
						{
							sum_xy=sum_xy+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][k];
							sum_xz=sum_xz+reconstruct_vector[(i+q+vector_size)%vector_size][j][(k+p+vector_size)%vector_size];
							sum_yz=sum_yz+reconstruct_vector[i][(j+p+vector_size)%vector_size][(k+p+vector_size)%vector_size];
							for (g=-1;g!=2;g++)
							{
								sum_3D=sum_3D+reconstruct_vector[(i+q+vector_size)%vector_size][(j+p+vector_size)%vector_size][(k+g+vector_size)%vector_size];
							}
						}
					}
					if (sum_3D==27)
					{
						Pre_process_vector[i][j][k]=1;
					}
					else if (sum_3D==0)
					{
						Pre_process_vector[i][j][k]=0;
					}
					else
					{
						if (reconstruct_vector[i][j][k]==1 && (sum_yz!=9 || (sum_xy!=9 && sum_xz!=9)))
						{
							temporay_vector[i][j][k]=1;
						}
						else if (reconstruct_vector[i][j][k]==0 && (sum_yz!=0 || (sum_xy!=0 && sum_xz!=0)))
						{
							temporay_vector[i][j][k]=0;
						}
					}
				}
			}
		}
	}

	///ѡȡ�������ĺ�ɫ���ص�
	for (i=0;i!=vector_size;i++)
	{
		for (j=0;j!=vector_size;j++)
		{
			for (k=0;k!=vector_size;k++)
			{
				sum_xy=0;
				sum_xz=0;
				sum_yz=0;
				for (p=-1;p!=2;p++)
				{
					for (q=-1;q!=2;q++)
					{
						sum_xy=sum_xy+reconstruct_vector[(i+p+vector_size)%vector_size][(j+q+vector_size)%vector_size][k];
					}
				}
				for (p=-1;p!=2;p++)
				{
					for (q=-1;q!=2;q++)
					{
						sum_xz=sum_xz+reconstruct_vector[(i+p+vector_size)%vector_size][j][(k+q+vector_size)%vector_size];
					}
				}
				for (p=-1;p!=2;p++)
				{
					for (q=-1;q!=2;q++)
					{
						sum_yz=sum_yz+reconstruct_vector[i][(j+p+vector_size)%vector_size][(k+q+vector_size)%vector_size];
					}
				}
				if ((sum_xy==8 || sum_xz==8 || sum_yz==8 ) && reconstruct_vector[i][j][k]==0 && Pre_process_vector[i][j][k]==2)
				{
					temporay_vector[i][j][k]=0;
				}
			}
		}
	}
	//temporay_vector���ѡ�񽻻���
	//Sign_exchange_vector��Ž����ɹ�������
	//Pre_process_vector����Ѿ�ȷ���ĵ㣬���ٲ��뽻��

	//////////////////////////////////////////////////////////ѡȡ������
	white_point_number=0;
	black_point_number=0;
	bool judgement=false; 

	for (i=0;i!=vector_size;i++)
	{
		for (j=0;j!=vector_size;j++)
		{
			for (k=0;k!=vector_size;k++)
			{
				if (temporay_vector[i][j][k]==1)
				{
					white_point_x.push_back(i);
					white_point_y.push_back(j);
					white_point_z.push_back(k);
					++white_point_number;
				}
				else if (temporay_vector[i][j][k]==0)
				{
					black_point_x.push_back(i);
					black_point_y.push_back(j);
					black_point_z.push_back(k);
					++black_point_number;
				}
			}
		}
	}

	class_white_point_number=white_point_number;
	class_black_point_number=black_point_number;
	temporay_vector.clear();
}

//190211
bool CAnnealing::get_filelist_from_dir(string path, vector<string>& files)
{
	long hFile=0;
	struct _finddata_t fileinfo;
	files.clear();
	if ((hFile=_findfirst(path.c_str(),&fileinfo))!=-1)
	{
		do 
		{
			if (!(fileinfo.attrib& _A_SUBDIR))
			{
				files.push_back(fileinfo.name);
			}
		} while (_findnext(hFile,&fileinfo)==0);
		_findclose(hFile);
		return true;
	}
	else
		return false;
}
