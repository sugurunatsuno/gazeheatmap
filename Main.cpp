
# include <Siv3D.hpp>

#include <sstream>
#include <vector>
#include <string>
#include <exception>
#include <Windows.h>

class Directory
{
public:
	static std::vector<std::string> GetFiles(const std::string& dir_path);
	static std::vector<std::string> GetFiles(const std::string& dir_path, const std::string& filter);
};

//imageとmatの変換
/*Image imageFromMat(cv::Mat_<cv::Vec4b> mat) {

	int width = mat.cols;
	int height = mat.rows;

	Image image;

	image.resize(width, height, Palette::Black);

	Color* pDst = image[0];
	const Color* pDstEnd = pDst + image.num_pixels;
	uint8* pSrc = mat.data;

	while (pDst != pDstEnd)
	{
		pDst->r = *(pSrc++);
		pDst->g = *(pSrc++);
		pDst->b = *(pSrc++);
		*(pSrc++);
		++pDst;
	}

	return image;

}

cv::Mat_<cv::Vec4b> matFromImage(Image &image) {
	cv::Mat_<cv::Vec4b> matDst(image.height, image.width, const_cast<cv::Vec4b*>(static_cast<const cv::Vec4b*>(image.data())), image.stride);

	return matDst;
}
*/

String filename_return(String &str) {
	auto rev = str.reverse();
	rev = rev.substr(4);
	auto ret = rev.reverse();

	return ret;
}



void Main()
{
	int textsize = 10;
	Font f(textsize);
	Array<FilePath> items;
	bool flag = false;
	Window::Resize(800, 600);
	Window::SetTitle(L"ヒートマップ作成ツール");

	String expression;
	int32 times = 0;
	if (!TobiiEyeX::Start())
	{
		f.draw(L"Tobii EyeXがないよ");
		WaitKey();
		return;
	}

	while (System::Update()) {
		f.draw(L"時間を秒で入力してね\n");
		
		
		if (Input::KeyQ.clicked)
		{
			return;
		}

		Input::GetCharsHelper(expression);
		f(expression).draw(0, textsize + 5);

		times = Parse<int>(expression);
		if (Input::KeyEnter.clicked) {
			break;
		}
	}




	while (System::Update()) 
	{
		if (Dragdrop::HasItems())
		{
			items = Dragdrop::GetFilePaths();
			flag = true;
			
		}
		else if (!flag){
			f.draw(L"ディレクトリをドロップしてね");
		}
		else {

			f(*(items.begin()) + L" でいい？ \nOKならEnterを、そうでないならディレクトリをドロップ \n終了ならQキー\n\n作業説明\n", times, L"秒間画像を見て、それを繰り返すだけ\n終わりにするならQキー\n視線の情報は" + *(items.begin()) + L"の中の同名のcsvファイルに保存").draw();
		}

		if (Input::KeyEnter.clicked && flag)
		{
			break;
		}
		if (Input::KeyQ.clicked)
		{
			return;
		}


	}



	String dir = *(items.begin());
	std::vector<std::string> path = Directory::GetFiles(dir.narrow());
	
	auto it = path.begin();
	String filename = dir + Widen(*it).str();
	String _name = Widen(*it).str();
	_name = filename_return(_name);

	CSVWriter writer(dir + _name + L".csv");

	EyeXState state;

	Image icon(filename);

	Window::Resize(icon.width, icon.height);
	Stopwatch stopwatch(true);

	Vec2 pos(0, 0);

	Texture t(icon);

	Window::SetTitle(L"ヒートマップ作成ツール");

	while (System::Update())
	{
		if (TobiiEyeX::HasNewState())
		{
			TobiiEyeX::GetState(state);
			pos = state.clientGazePos;

		}
		t.draw();
		writer.write(pos.x);
		writer.write(pos.y);
		writer.nextLine();



		if (Input::KeyEnter.clicked || stopwatch.s() == times) {
			it++;

			if (it == path.end() ) {
				break;
			}

			writer.close();
			

			filename = dir + Widen(*it).str();
			_name = Widen(*it).str();
			_name = filename_return(_name);
			writer.open(dir + _name + L".csv");
			icon = Image(filename);
			t = Texture(icon);
			t.draw();
			Window::Resize(icon.width, icon.height);
			stopwatch.restart();
		}
		if (Input::KeyQ.clicked) {
			return;
		}
	}
}




//! 指定フォルダのファイルを取得する
//! ex: std::vector<std::string> path = util::Directory::GetFiles("C:\\aa\\");
inline
std::vector<std::string> Directory::GetFiles(const std::string& dir_path)
{
	return GetFiles(dir_path, "*.*");
}

//! 指定フォルダのファイルを取得する
//! ex: std::vector<std::string> path = util::Directory::GetFiles("C:\\aa\\", "*.png");
inline
std::vector<std::string> Directory::GetFiles(const std::string& dir_path, const std::string& filter)
{
	WIN32_FIND_DATAA fd;

	std::string ss = dir_path + filter;
	HANDLE hFind = FindFirstFileA(ss.c_str(), &fd);

	// 検索失敗
	if (hFind == INVALID_HANDLE_VALUE)
	{
		throw std::exception("util::Directory::GetFiles failed");
	}

	std::vector<std::string> fileList;
	do
	{
		// フォルダは除く
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		// 隠しファイルは除く
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			continue;

		fileList.push_back(fd.cFileName);
	} while (FindNextFileA(hFind, &fd));

	FindClose(hFind);

	return fileList;
}