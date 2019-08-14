#include "stdafx.h"
#include "io.h"
#include "Annealing.h"
#include  <vector>
#include <fstream>
#include <atlstr.h>
using namespace std;
using namespace cv;

   //使用CFileDialog类的头文件

CAnnealing::CAnnealing(void)
{
}

CAnnealing::~CAnnealing(void)
{
}

//size_number(模板大小对应编码的最大值)初始化
//Rec_template_size=3
//Inital_image_vector存放原图的容器
//big_3Dimg存放大孔三维结构
//beishu初始化
CAnnealing::CAnnealing(IplImage* img, int width, int height,int template_size)
{
	image = new uchar[width*height];
	class_white_point_number=0;
	class_black_point_number=0;
	grid_size=0;
	vector_size=0;
	inital_height=height;              //将读入原图的尺寸赋给这两个变量，供所有的函数使用
	inital_width=width;                //将读入原图的尺寸赋给这两个变量，供所有的函数使用，
	Rec_template_size=template_size;   //析构函数传递采用模板大小参数。
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
	cout<<"请输入期望重建的图像尺寸(与训练图像尺寸成整倍数关系)："<<endl;
	cin>>final_imgsize;
	if (final_imgsize<height)
	{
		cout<<"重建尺寸小于训练图像！"<<endl;
		return;
	}
	beishu=final_imgsize/height;     //计算待重建尺寸与训练图像之间的倍数关系

	//////////////////////////190211：保存低分辨率大孔三维结构////////////////////////////
	int b_layer;
	cout<<"请输入大孔(低分辨率)三维结构的层数："<<endl;
	cin>>b_layer;

	if (final_imgsize!=b_layer)
	{
		cout<<"小孔融合尺寸与大孔结构尺寸不相等！"<<endl;
		return;
	}

	big_3Dimg.resize(b_layer);
	AfxSetResourceHandle(GetModuleHandle(NULL));        //在控制台程序中使用CFileDialog类必须要这句
	AfxMessageBox(_T("请打开大孔(低分辨率)三维结构“黑白”序列图所在文件夹！"));
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
			AfxMessageBox(_T("文件读取出错！"));
		}
		
		//保存序列图到容器
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
					int t=0;         //防止读入的图像孔点不是白色！
					if (p[x]!=0)
					{
						t=255;
					}
					big_3Dimg[z][y][x]=t;
				}
			}
		}
	}
	////以下为了测试
	//for (int y = 0; y < 256; y++)
	//{
	//	for (int x = 0; x < 256; x++)
	//	{
	//		cout << big_3Dimg[1][y][x] << endl;
	//	}
	//}
	////cout << "save successfully" << endl;
}

/////////////////////////////////1、确定最小网格/////////////////////////////////////////
void CAnnealing::Decide_min_grid(int& min_grid)
{
	int i=0,j=0,p=0,q=0,sum=0,k=0,w=0;
	int white_point=0,black_point=0;
	int loop_time=log((float)inital_width)/log(2.0);
	int temporay_value_1=0,temporay_value_2=0;
	bool judgement=false;

	for (k=0;k!=loop_time-1;k++)////最小网格从4*4开始
	{
		temporay_value_1=(int)pow(2.0,k);                   ////相邻网格点之间的距离
		temporay_value_2=inital_width/temporay_value_1;     ////网格的大小
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
		for (i=1;i!=temporay_value_2-1;i++)      //3*3模板的中心点，判断分级图像是否存在3*3区域为同一相，只要存在，则继续寻找
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

//inital_rand_data(初始最小容器)初始化(190113改)->(190212：将大孔三维结构信息也保存到最小容器中)
//大孔孔点所在位置初始化为300！——190212——重建的背景点可以在大孔孔点所在的位置，这样既能保证重建结果的孔隙度正确，且保证大孔结构不受影响
//大孔孔点位置只用限制为小孔孔点不能重建，但可以是小孔背景点（大孔孔点固定，不能被随机赋值）
void CAnnealing::Set_inital_data()
{
	//先定义一个三维容器，用于存放初始最小网格
	int i=0,j=0,k=0,inital_white_point=0;

	int new_inital_size=Rec_min_grid_size*beishu;     //190113
	int size_number=inital_height/Rec_min_grid_size;   
	int b_pnum=0;   //大孔孔点数                      //190212

	//inital_rand_data初始化为2和300（之前全是2）
	inital_rand_data.resize(new_inital_size);      //inital_rand_data初始最小容器随机赋值，
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
					inital_rand_data[i][j][k]=300;      //190212：大孔孔点所在位置
					++b_pnum;
				}
				else
					inital_rand_data[i][j][k]=2;
			}
		}
	}
	//按照孔隙度进行随机赋初值，白色用1表示，黑色用0表示
	for (i=0;i!=Rec_min_grid_size;i++)
	{
		for (j=0;j!=Rec_min_grid_size;j++)
		{
			if (Inital_image_vector[i*size_number][j*size_number]==1)    //Inital_image_vector存放原图容器（高分辨）
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
	//原图的孔隙点数相当于待重建三维结构一层的孔隙点数，乘以层数就为整个三维结构孔隙度
	for (i=0;i!=new_inital_size*inital_white_point*(beishu*beishu);i++)   
	{
		rand_site= rand32()%counter_number;
		if (inital_rand_data[point_x[rand_site]][point_y[rand_site]][point_z[rand_site]]!=300)   //190212
		{
			inital_rand_data[point_x[rand_site]][point_y[rand_site]][point_z[rand_site]]=1;//1表示白色点，0表示黑色点
			site_x= point_x.begin()+rand_site;   //找到已经随机赋值的点，并删除，避免重复赋值
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
			site_x= point_x.begin()+rand_site;   //190213：找到“大孔孔点”所在位置，并删除，避免重复赋值
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

//为(new)vector_size(当前级训练图像的尺寸)赋值
void CAnnealing::Transfer_grid_size(int T_grid_size)
{
	grid_size=T_grid_size;
	vector_size=Rec_min_grid_size*(int)pow(2.0,grid_size);
	new_vector_size=vector_size*beishu;   //190113
}

//TI_code_value(存放训练图像的多点密度函数)
void CAnnealing::TI_Multi_point_density_function_p()
{
	TI_code_value.clear();   //TI_code_value存放训练图像的多点密度函数             //将上一网格存储的多点密度函数清空
	int i=0,j=0;
	int reduce_size=inital_height/vector_size;//缩小倍数
	int k=Rec_template_size/2;
	vector<vector<int>> Temporary_vector;
	Temporary_vector.resize(vector_size);    //保存当前级数的TI
	for(i=0;i!=vector_size;++i)
	{
		Temporary_vector[i].resize(vector_size);
	}

	//将对应点的原始图像放到中间过程容器
	for (i=0;i!=vector_size;i++)
	{
		for (j=0;j!=vector_size;j++)
		{
			if (Inital_image_vector[i*reduce_size][j*reduce_size]==1)
			{
				Temporary_vector[i][j]=1;//为了后面编码方便将255值变为1
			}
			else
			{
				Temporary_vector[i][j]=0;
			}
		}
	}

	//计算此时缩小图像的多点密度函数
	int code_value=0;//模式编码值
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
					//这种编码方式能够保证编码不重复吗？最大编码值为511
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
	IplImage* pImage=cvCreateImage(cvSize(vector_size,vector_size), IPL_DEPTH_8U, 1);//存储图片	
	int step =pImage->widthStep;  //每幅图像每行字节数
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
	//cout<<"原始白色点数:"<<white_point<<endl;
	char name[100];
	sprintf(name,"The inital%d!.bmp",grid_size);   //grid_size就是T_grid_size

	cvSaveImage(name, pImage);
	cvReleaseImage(&pImage);
}

//190212：reconstruct_vector第一次初始化时大孔孔点所在位置都初始化为0了！
void CAnnealing::Set_inital_to_reconstruct_uncondition_1()    //为最小网格对应的三维结构赋初值（即为Set_inital_data）
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
	/////////////////////////////////////////////////给判决容器赋初值，将上层图像传递给重建容器；
	reconstruct_vector.resize(new_vector_size);     //(190113)存放重建结果容器
	Pre_process_vector.resize(new_vector_size);     //存放预处理结果容器，标注3*3全为黑或者全白的区域 
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
					reconstruct_vector[i][j][k]=2;                            /////初始化中间容器"大孔孔点"为300
				}
				else
				{
					Pre_process_vector[i][j][k]=2;                              /////初始化中间容器为2
					reconstruct_vector[i][j][k]=2;                              /////初始化中间容器为2
				}
			}
		}
	}
	for (i=0;i!=vector_size;++i)
	{
		for (j=0;j!=vector_size;++j)
		{                             
			if (Inital_image_vector[i*distance_point][j*distance_point]==1)              /////计算本层原图白色点数
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
					reconstruct_vector[2*i][2*j][2*k]=inital_rand_data[i][j][k];              /////将上层重建的结果传递到对应网格点
					Pre_process_vector[2*i][2*j][2*k]=inital_rand_data[i][j][k];              /////将上层重建的结果传递到对应网格点
					if (inital_rand_data[i][j][k]==1)
					{
						up_grid_white_number++;                                               /////计算上层重建的结果孔隙相（190212：不与下层大孔孔点重合）的个数
					}
				}
				else
				{
					//if (inital_rand_data[i][j][k]==0)    //190213：大孔孔点所在的位置值也为0（去掉这一条件，将大孔孔
					//{                                             //点所在位置设为已知的背景条件点，对比实验结果！）
						reconstruct_vector[2*i][2*j][2*k]=0;                                      
					/*}*/
				}
			}
		}
	}

	/////////////////////////////////////////////////////
	for (i=0;i!=new_vector_size;++i)    //190113（处理奇数x、y、z对应的像素点！）
	{
		for (j=0;j!=new_vector_size;++j)
		{
			for (k=0;k!=new_vector_size;++k)
			{
				if (i%2==1 && j%2==1 && k%2==1)    //3*3模板中心点长宽高坐标都必须为奇数！
				{
					sum=0;                                                                    /////判断3*3*3区域是否全为1或者全0
					for (q=-1;q!=2;q++)    //i,j,k为奇数（奇+奇=偶、奇+偶=奇）
					{
						for (p=-1;p!=2;p++)
						{
							for (n=-1;n!=2;n++)    
							{
								//下面的条件需要所有公式（1）为偶数，(i+q+new_vector_size)%new_vector_size使用的策略是周期边界条件，防止越界！
								if (((i+q+new_vector_size)%new_vector_size)%2==0 && ((j+p+new_vector_size)%new_vector_size)%2==0 && ((k+n+new_vector_size)%new_vector_size)%2==0)
								{
									//q、p、n需要为奇数
									//满足条件的有(-1/1,-1/1,-1/1)共8个点，它们刚好为前一级映射过来的点
									sum=sum+reconstruct_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+n+new_vector_size)%new_vector_size];
								}
							}
						}
					}
					//注意：reconstruct_vector虽初始化值为2，但是一部分点（隔点插值0、2、4...）的值又根据前一级重建结果被初始化为0或1
					//这里sum是统计的前一级映射过来的像素点！
					if (sum==8)                          /////全1时，除该区域中心点外，其余点全赋值为1
					{
						for (q=-1;q!=2;q++)               //i,j,k为奇数（奇+奇=偶、奇+偶=奇）
						{
							for (p=-1;p!=2;p++)
							{
								for (n=-1;n!=2;n++)
								{
									//有一个公式满足和为奇数即可
									if (((i+q+new_vector_size)%new_vector_size)%2==1 || ((j+p+new_vector_size)%new_vector_size)%2==1 || ((k+n+new_vector_size)%new_vector_size)%2==1)
									{
										//q、p、n至少有一个为偶数，即0！
										//即满足由前一级映射过来的点全为1时，3*3*3模板中所有值为2的点（扩展点）
										if (Pre_process_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+n+new_vector_size)%new_vector_size]!=300)   //190212
										{
											Pre_process_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+n+new_vector_size)%new_vector_size]=31;
										}
									}
								}
							}
						}
					}
					else if (sum==0)  /////全0时，除该区域中心点外，其余点全赋值为0
					{
						for (q=-1;q!=2;q++)
						{
							for (p=-1;p!=2;p++)
							{
								for (n=-1;n!=2;n++)
								{
									if (((i+q+new_vector_size)%new_vector_size)%2==1 || ((j+p+new_vector_size)%new_vector_size)%2==1 || ((k+n+new_vector_size)%new_vector_size)%2==1)
									{
										//if (Pre_process_vector[(i+q+new_vector_size)%new_vector_size][(j+p+new_vector_size)%new_vector_size][(k+n+new_vector_size)%new_vector_size]!=300)    //190212：可以不要这个条件吧？大孔孔点可以在小孔结构的背景处
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
	//current_grid_white_number为本级训练图像中的白点数(190113)，up_grid_white_number为前一级重建结果中的白点数
	all_add_white_number=new_vector_size*current_grid_white_number*(beishu*beishu)-up_grid_white_number;         /////本级网格需要添加白色点个数
	all_add_black_number=new_vector_size*(new_vector_size*new_vector_size-current_grid_white_number*(beishu*beishu));
	int white_point_number_3D=0;
	int black_point_number_3D=0;
	int white_point_number_2D=0;
	int black_point_number_2D=0;
	//int white_point_number_1D=0;
	//int black_point_number_1D=0;

	for (i=0;i!=new_vector_size;++i)         /////统计上面添加1和0的个数
	{
		for (j=0;j!=new_vector_size;++j)
		{
			for (k=0;k!=new_vector_size;++k)
			{
				//Pre_process_vector中为黑白点可能存在的位置
				//根据相的聚合规律，前一级全为白点/黑点的模式，在下一级的扩展模式中扩展点大概率也是白点/黑点
				if (Pre_process_vector[i][j][k]==31)           //3*3*3区域全为1
				{
					white_point_number_3D++;
				}
				else if (Pre_process_vector[i][j][k]==30)      //3*3*3区域全为0
				{
					black_point_number_3D++;
				}
			}
		}
	}

	int point_x=0;
	int point_y=0;
	int point_z=0;

	//若实际待添加的黑白点数>=根据聚合规律假设的黑白点数，则直接将假设的黑白点位置直接按假设赋值
	if (all_add_white_number>=white_point_number_3D && all_add_black_number>=black_point_number_3D) /////将reconstruct_vector对应3D网格点赋值1和0
	{
		for (i=0;i!=new_vector_size;++i)//先将3*3*3区域判断的点直接将这些满足条件的数据赋值
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
		//下面：处理偶数层
		////////////////////////////////////////////////////////////////////////////以上部分是处理上级网格3*3*3区域为全1和全0情况
		////////////////////////////////////////////////////////////////////////////以下部分是处理上级网格2*2区域为全1和全0情况
		for (i=0;i!=new_vector_size;++i)//判断全1或者全0的2*2*2区域点个数
		{
			for (j=0;j!=new_vector_size;++j)
			{
				for (k=0;k!=new_vector_size;++k)
				{
					//i为偶数，j、k为奇数，且像素点还未赋值（仍是初始值2||(190212：300)）
					if (i%2==0 && j%2==1 && k%2==1 && (Pre_process_vector[i][j][k]==2||Pre_process_vector[i][j][k]==300) )
					{
						sum=0;
						for (q=-1;q!=2;q++)     
						{
							for (p=-1;p!=2;p++)
							{
								//只考虑j、k，需要q、p为奇数
								if (((j+q+new_vector_size)%new_vector_size)%2==0 && ((k+p+new_vector_size)%new_vector_size)%2==0)
								{
									sum=sum+reconstruct_vector[i][(j+q+new_vector_size)%new_vector_size][(k+p+new_vector_size)%new_vector_size];
								}
							}
						}
						if (sum==4)
						{
							if (Pre_process_vector[i][j][k]!=300)    //190213：是否有点问题？Pre大孔所在的位置可能在上一部分已经赋值为30？！
							{                                                  //没问题！因为上面控制了Pre的值只能为2或300！
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

					//k为偶数，j、i为奇数，且像素点还未赋值（仍是初始值2||(190212：300)）
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

					//j为偶数，k、i为奇数，且像素点还未赋值（仍是初始值2||(190212：300)）
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
		/////第二次2*2中心点直接赋值满足孔隙度
		if (all_add_white_number>=(white_point_number_3D+white_point_number_2D) && all_add_black_number>=(black_point_number_3D+black_point_number_2D))
		{
			for (i=0;i!=new_vector_size;++i)//先直接将这些满足条件的数据赋值
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

			///////////////第三次判断2*2区域是否全为0或者1
			add_white_number_2D=(all_add_white_number-white_point_number_3D-white_point_number_2D);//然后从剩余部分随机赋值
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

		else /////第二次2*2*2中心点直接赋值不满足孔隙度时，回退到上一步3*3*3设置状态
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

	else   /////直接按照需要随机赋值
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
					Pre_process_vector[i][j][k]=reconstruct_vector[i][j][k];/////最后将Pre_process_vector作为指示容器
				}
			}
		}
	}
	/////将Pre_process_vector设置为指示器
	for (i=0;i!=new_vector_size;i++)
	{
		for(j=0;j!=new_vector_size;j++)
		{
			for (k=0;k!=new_vector_size;k++)
			{
				if (Pre_process_vector[i][j][k]!=2 && Pre_process_vector[i][j][k]!=300 && reconstruct_vector[i][j][k]==1)
				{                                     //190213：该条件好像不需要，因为reconst为1的位置一定不是大孔孔点位置
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

//Reconstruct_code_value(存放重建图像的多点密度函数)——190212：reconstruct_vector中没有300值！
void CAnnealing::Recstruct_Multi_point_density_function_p()
{
	Reconstruct_code_value.clear();//将上一网格存储的多点密度函数清空  
	int i=0,j=0,k=0,q=0,p=0,r=0;
	int s=Rec_template_size/2;
	pair<map<int,int>::iterator,bool> pair_code_value;//计算此时重建图像的多点密度函数
	int code_value=0;//模式编码值
	for (k=0;k!=new_vector_size;++k)  //对三个面的模式密度进行统计
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
	for (i=0;i!=size_number;i++) //size_number模板对应大小编码最大值
	{
		//(190113)3表示x、y、z三个面，vector_size为三维层数，TI_code_value为单层的模式密度函数
		E_differ=E_differ+abs(3*new_vector_size*TI_code_value[i]*(beishu*beishu)-Reconstruct_code_value[i]);
	}
	Enerry_origial=E_differ;
}

//white_point_number=Main_white_point_number
//190213：由于reconstruct_vector中没有对大孔孔点的标志(300)，必须限定大孔孔点位置只能为背景点且不能参与交换！
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
				if (reconstruct_vector[i][j][k]==1)    //存放重建结果容器
				{
					white_point_x.push_back(i);
					white_point_y.push_back(j);
					white_point_z.push_back(k);
					++white_point_number;
				}
				else if (big_3Dimg[i*distance_point][j*distance_point][k*distance_point]==0 && reconstruct_vector[i][j][k]==0)
				{                  //190213：只有非大孔孔点所在位置的背景点才能参与交换！
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

//temporay_vector存放选择交换点
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

	///判断将上层网格点对应当前网格点像素值相反的网格点作为交换点，并且排除上层作为全零和全1的情况
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
				if (i%2==1 && j%2==1 && k%2==1 && (Pre_process_vector[i][j][k]==2/*||Pre_process_vector[i][j][k]==300*/) ) //存放预处理结果容器，标注3*3全为黑或全白的区域
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
							Pre_process_vector[i][j][k]=1;   //变成固定点
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
							if (Pre_process_vector[i][j][k]!=300)   //190212：大孔孔点位置不允许交换（只能为小孔的背景点）   //光看这一句会觉得：有些300变成20/30了呀？但大前提if限定了pre为2/300
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
							if (Pre_process_vector[i][j][k]!=300)   //190212：大孔孔点位置不允许交换（只能为小孔的背景点）
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
							if (Pre_process_vector[i][j][k]!=300)   //190212：大孔孔点位置不允许交换（只能为小孔的背景点）
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
							if (Pre_process_vector[i][j][k]!=300)   //190212：大孔孔点位置不允许交换（只能为小孔的背景点）
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
							if (Pre_process_vector[i][j][k]!=300)   //190212：大孔孔点位置不允许交换（只能为小孔的背景点）
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
							if (Pre_process_vector[i][j][k]!=300)   //190212：大孔孔点位置不允许交换（只能为小孔的背景点）
							{
								temporay_vector[i][j][k]=0;
							}
						}
					}
				}
			}
		}
	}
	//temporay_vector存放选择交换点（190212：大孔孔点的位置不能参数交换）
	//Sign_exchange_vector存放交换成功与否情况
	//Pre_process_vector存放已经确定的点，不再参与交换

	//////////////////////////////////////////////////////////选取交换点
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

//随机交换的原理？？
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

//white/black_rand_site 白/黑色交换点数位置
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
	int code_value=0;//模式编码值
	int q=0,p=0;
	int k=Rec_template_size/2;
	int white_x=white_point_x[white_rand_site];
	int white_y=white_point_y[white_rand_site];
	int white_z=white_point_z[white_rand_site];
	int black_x=black_point_x[black_rand_site];
	int black_y=black_point_y[black_rand_site];
	int black_z=black_point_z[black_rand_site];
	pair<map<int,int>::iterator,bool> pair_code_value;

	//这里首先将Exchange_code_value_white/black清空了，这样在执行下面的--操作时，为什么不会出错？
	Exchange_code_value_white.clear();
	Exchange_code_value_black.clear();
	for (q=-k;q!=k+1;q++)      ////次循环为每个交换点决定的9个模式
	{
		for (p=-k;p!=k+1;p++)
		{
			reconstruct_vector[white_x][white_y][white_z]=1;//交换前(190212：坐标是由temporay_vector存放选择的交换点确定的)
			reconstruct_vector[black_x][black_y][black_z]=0;

			////////////////////////////////////////////////////XY平面交换点决定的模式

			code_value=0;
			for (i=-k;i!=k+1;i++) ////此循环为计算每个模式
			{
				for (j=-k;j!=k+1;j++)
				{
					code_value=code_value*2+reconstruct_vector[(i+white_x+q+new_vector_size)%new_vector_size][(j+white_y+p+new_vector_size)%new_vector_size][white_z];
				}
			}
			--Exchange_code_value_white[code_value];//减去交换前由白色交换点确定的模式


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
				--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
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
				--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
			}

			////////////////////////////////////////////////////XZ平面交换点决定的模式
			code_value=0;
			for (i=-k;i!=k+1;i++)
			{
				for (j=-k;j!=k+1;j++)
				{
					code_value=code_value*2+reconstruct_vector[(i+white_x+q+new_vector_size)%new_vector_size][white_y][(j+white_z+p+new_vector_size)%new_vector_size];
				}
			}
			--Exchange_code_value_white[code_value];//减去交换前由白色交换点确定的模式

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
				--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
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
				--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
			}

			//////////////////////////////////////////////////YZ平面交换点决定的模式
			code_value=0;
			for (i=-k;i!=k+1;i++)
			{
				for (j=-k;j!=k+1;j++)
				{
					code_value=code_value*2+reconstruct_vector[white_x][(i+white_y+q+new_vector_size)%new_vector_size][(j+white_z+p+new_vector_size)%new_vector_size];
				}
			}
			--Exchange_code_value_white[code_value];//减去交换前由白色交换点确定的模式

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
				--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
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
				--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			reconstruct_vector[white_x][white_y][white_z]=0;//交换后
			reconstruct_vector[black_x][black_y][black_z]=1;
			////////////////////////////////////////////////////XY平面交换点决定的模式
			code_value=0;
			for (i=-k;i!=k+1;++i)
			{
				for (j=-k;j!=k+1;++j)
				{
					code_value=code_value*2+reconstruct_vector[(i+white_x+q+new_vector_size)%new_vector_size][(j+white_y+p+new_vector_size)%new_vector_size][white_z];
				}
			}
			pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//加上交换前由白色交换点确定的模式
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
				if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
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
				if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
				{
					++pair_code_value.first->second;
				}
			}

			////////////////////////////////////////////////////YZ平面交换点决定的模式
			code_value=0;
			for (i=-k;i!=k+1;++i)
			{
				for (j=-k;j!=k+1;++j)
				{
					code_value=code_value*2+reconstruct_vector[white_x][(j+white_y+p+new_vector_size)%new_vector_size][(i+white_z+q+new_vector_size)%new_vector_size];
				}
			}
			pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//加上交换前由白色交换点确定的模式
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
				if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
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
				if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
				{
					++pair_code_value.first->second;
				}
			}


			////////////////////////////////////////////////////XZ平面交换点决定的模式
			code_value=0;
			for (i=-k;i!=k+1;++i)
			{
				for (j=-k;j!=k+1;++j)
				{
					code_value=code_value*2+reconstruct_vector[(i+white_x+q+new_vector_size)%new_vector_size][white_y][(j+white_z+p+new_vector_size)%new_vector_size];
				}
			}
			pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//加上交换前由白色交换点确定的模式
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
				if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
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
				if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
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
		Engerry_before_exchange_white=Engerry_before_exchange_white+abs(Reconstruct_code_value[iter_Success->first]-3*new_vector_size*TI_code_value[iter_Success->first]*(beishu*beishu));//计算没有变化的模式个数
		Reconstruct_code_value[iter_Success->first]=Reconstruct_code_value[iter_Success->first]+Exchange_code_value_white[iter_Success->first];//保存原来的模式
		Engerry_after_exchange_white=Engerry_after_exchange_white+abs(Reconstruct_code_value[iter_Success->first]-3*new_vector_size*TI_code_value[iter_Success->first]*(beishu*beishu));//计算模式差值
	}
	Engerry_change_white=Engerry_after_exchange_white-Engerry_before_exchange_white;

	iter_Success = Exchange_code_value_black.begin();
	for(iter_Success;iter_Success!=Exchange_code_value_black.end();++iter_Success)
	{
		Engerry_before_exchange_black=Engerry_before_exchange_black+abs(Reconstruct_code_value[iter_Success->first]-3*new_vector_size*TI_code_value[iter_Success->first]*(beishu*beishu));//计算没有变化的模式个数
		Reconstruct_code_value[iter_Success->first]=Reconstruct_code_value[iter_Success->first]+Exchange_code_value_black[iter_Success->first];//保存原来的模式
		Engerry_after_exchange_black=Engerry_after_exchange_black+abs(Reconstruct_code_value[iter_Success->first]-3*new_vector_size*TI_code_value[iter_Success->first]*(beishu*beishu));//计算模式差值
	}
	Engerry_change_black=Engerry_after_exchange_black-Engerry_before_exchange_black;

	Enerry_Reverse=Enerry_origial;//将交换前的能量差保存，以便替换回来
	Engerry_down=Enerry_origial+Engerry_change_white+Engerry_change_black;  //传递交换后的能量差
	Enerry_origial=Engerry_down;//将当期能量差作为新状态
	reconstruct_vector[white_x][white_y][white_z]=0;//交换后
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
		IplImage* pImage=cvCreateImage(cvSize(new_vector_size,new_vector_size), IPL_DEPTH_8U, 1);//存储图片	
		int step =pImage->widthStep;  //每幅图像每行字节数
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
		//cout<<"重建后白色点数:"<<white_point<<endl;
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
		cvSaveImage(name, pImage);//最终图片名格式为数字编号，例“1.bmp”
		cvReleaseImage(&pImage);
	}
}

//190213：保存包含大孔的三维孔隙结构
void CAnnealing::Set_reconstruct_to_final()
{
	int i=0,j=0,k=0;
	if (final_imgsize!=new_vector_size)
	{
		cout<<"重建尺寸出错！"<<endl;
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
	AfxSetResourceHandle(GetModuleHandle(NULL));       ////在控制台程序中使用CFileDialog类必须要这句
	AfxMessageBox(_T("请选择保存融合结果的文件夹！"));

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
	cout<<"重建结果保存成功！"<<endl;
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void CAnnealing::TI_Multi_point_density_function()
{
	TI_code_value.clear();//将上一网格存储的多点密度函数清空
	int i=0,j=0;
	int reduce_size=inital_height/vector_size;//缩小倍数
	int k=Rec_template_size/2;
	vector<vector<int>> Temporary_vector;
	Temporary_vector.resize(vector_size);
	for(i=0;i!=vector_size;++i)
	{
		Temporary_vector[i].resize(vector_size);
	}

	//将对应点的原始图像放到中间过程容器
	for (i=0;i!=vector_size;i++)
	{
		for (j=0;j!=vector_size;j++)
		{
			if (Inital_image_vector[i*reduce_size][j*reduce_size]==1)
			{
				Temporary_vector[i][j]=1;//为了后面编码方便将255值变为1
			}
			else
			{
				Temporary_vector[i][j]=0;
			}
		}
	}

	//计算此时缩小图像的多点密度函数
	int code_value=0;//模式编码值
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
	Reconstruct_code_value.clear();//将上一网格存储的多点密度函数清空  
	int i=0,j=0,k=0,q=0,p=0,r=0;
	int s=Rec_template_size/2;
	pair<map<int,int>::iterator,bool> pair_code_value;//计算此时重建图像的多点密度函数
	int code_value=0;//模式编码值
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
	int code_value=0;//模式编码值
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
			reconstruct_vector[white_x][white_y][white_z]=1;//交换前
			reconstruct_vector[black_x][black_y][black_z]=0;
			////////////////////////////////////////////////////XY平面交换点决定的模式
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
				--Exchange_code_value_white[code_value];//减去交换前由白色交换点确定的模式
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
					--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
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
					--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
				}
			}
			////////////////////////////////////////////////////XZ平面交换点决定的模式
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
				--Exchange_code_value_white[code_value];//减去交换前由白色交换点确定的模式
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
					--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
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
					--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
				}
			}
			//////////////////////////////////////////////////YZ平面交换点决定的模式
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
				--Exchange_code_value_white[code_value];//减去交换前由白色交换点确定的模式
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
					--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
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
					--Exchange_code_value_black[code_value];//减去交换前由黑色交换点确定的模式
				}
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			reconstruct_vector[white_x][white_y][white_z]=0;//交换后
			reconstruct_vector[black_x][black_y][black_z]=1;
			////////////////////////////////////////////////////XY平面交换点决定的模式
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
				pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//加上交换前由白色交换点确定的模式
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
					if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
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
					if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
					{
						++pair_code_value.first->second;
					}
				}
			}

			////////////////////////////////////////////////////YZ平面交换点决定的模式
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
				pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//加上交换前由白色交换点确定的模式
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
					if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
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
					if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
					{
						++pair_code_value.first->second;
					}
				}
			}

			////////////////////////////////////////////////////XZ平面交换点决定的模式
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
				pair_code_value=Exchange_code_value_white.insert(make_pair(code_value,1));//加上交换前由白色交换点确定的模式
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
					if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
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
					if(!pair_code_value.second)           //加上交换前由黑色交换点确定的模式
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
		Engerry_before_exchange_white=Engerry_before_exchange_white+abs(Reconstruct_code_value[iter_Success->first]-3*vector_size*TI_code_value[iter_Success->first]);//计算没有变化的模式个数
		Reconstruct_code_value[iter_Success->first]=Reconstruct_code_value[iter_Success->first]+Exchange_code_value_white[iter_Success->first];//保存原来的模式
		Engerry_after_exchange_white=Engerry_after_exchange_white+abs(Reconstruct_code_value[iter_Success->first]-3*vector_size*TI_code_value[iter_Success->first]);//计算模式差值
	}
	Engerry_change_white=Engerry_after_exchange_white-Engerry_before_exchange_white;

	iter_Success = Exchange_code_value_black.begin();
	for(iter_Success;iter_Success!=Exchange_code_value_black.end();++iter_Success)
	{
		Engerry_before_exchange_black=Engerry_before_exchange_black+abs(Reconstruct_code_value[iter_Success->first]-3*vector_size*TI_code_value[iter_Success->first]);//计算没有变化的模式个数
		Reconstruct_code_value[iter_Success->first]=Reconstruct_code_value[iter_Success->first]+Exchange_code_value_black[iter_Success->first];//保存原来的模式
		Engerry_after_exchange_black=Engerry_after_exchange_black+abs(Reconstruct_code_value[iter_Success->first]-3*vector_size*TI_code_value[iter_Success->first]);//计算模式差值
	}
	Engerry_change_black=Engerry_after_exchange_black-Engerry_before_exchange_black;

	Enerry_Reverse=Enerry_origial;//将交换前的能量差保存，以便替换回来
	Engerry_down=Enerry_origial+Engerry_change_white+Engerry_change_black;  //传递交换后的能量差
	Enerry_origial=Engerry_down;//将当期能量差作为新状态
	reconstruct_vector[white_x][white_y][white_z]=0;//交换后
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

	///判断将上层网格点对应当前网格点像素值相反的网格点作为交换点，并且排除上层作为全零和全1的情况
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

	//////////////////////////////////////////////////////////选取交换点
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
	/////////////////////////////////////////////////给判决容器赋初值，将上层图像传递给重建容器；
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
					reconstruct_vector[i][j][k]=inital_rand_data[i/2][j/2][k/2];     /////将上层重建的结果传递到对应网格点
					if (inital_rand_data[i/2][j/2][k/2]==1)
					{
						up_white_number++;
					}
				}
				else
				{
					reconstruct_vector[i][j][k]=2;                        /////其余点赋初值2
				}
			}
		}
	}

	for (i=0;i!=vector_size;++i)
	{
		for (j=0;j!=vector_size;++j)
		{
			if (Inital_image_vector[i*distance_point][j*distance_point]==1)/////统计对应网格白色点数
			{
				inital_white_number++;
			}
		}
	}
	add_white_number=vector_size*inital_white_number-up_white_number;
	int point_x=0;
	int point_y=0;
	int point_z=0;
	/////////////////////////////////////////////////////////给重建容器其余位置按照孔隙度随机赋值；
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
//	///相邻网格反相选点法，且排除上层作为全零和全1的情况
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
//	///选取不连续的黑色像素点
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
//	//////////////////////////////////////////////////////////选取交换点
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

	///判断将上层网格点对应当前网格点像素值相反的网格点作为交换点，并且排除上层作为全零和全1的情况
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

	///选取不连续的黑色像素点
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
	//temporay_vector存放选择交换点
	//Sign_exchange_vector存放交换成功与否情况
	//Pre_process_vector存放已经确定的点，不再参与交换

	//////////////////////////////////////////////////////////选取交换点
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
