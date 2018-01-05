#include <cgnslib.h>
#include <iriclib.h>
#include <stdio.h>
#include <stdlib.h>

#include <QProcess>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void load_elevation(const std::string& cgns_fname, int* nx, int* ny, std::vector<double>* elev);
void gen_input(const std::string& dem_fname, int nx, int ny, const std::vector<double>& elev);
void launch_mesh1v3(const std::string& demname, const std::string& aacname, const std::string& dirname);
void load_result(const std::string& outname, int nx, int ny, std::vector<int>* val);
void output_attr(const std::string& cgns_fname, const std::string& attname, std::vector<int>& val);

int main(int argc, char* argv[])
{
    std::string cgnsname = argv[1];
	int baseId = atoi(argv[2]);
	int zoneId = atoi(argv[3]);
	std::string attname = argv[4];
	std::string demname = "dem.txt";
	std::string aacname = "aac.txt";
	std::string dirname = "dir.txt";

	int nx, ny;
	std::vector<double> elev;

	load_elevation(cgnsname, &nx, &ny, &elev);
	gen_input(demname, nx, ny, elev);
	launch_mesh1v3(demname, aacname, dirname);

	std::vector<int> result;
	load_result(dirname, nx, ny, &result);
	output_attr(cgnsname, attname, result);

	unlink(demname.c_str());
	unlink(aacname.c_str());
	unlink(dirname.c_str());
}

void load_elevation(const std::string& cgns_fname, int* nx, int* ny, std::vector<double>* elev)
{
    int fn, ier;

    ier = cg_open(cgns_fname.c_str(), CG_MODE_READ, &fn);
    ier = cg_iRIC_InitRead(fn);

    cgsize_t isize, jsize;
    ier = cg_iRIC_GotoGridCoord2d(&isize, &jsize);
    *nx = isize - 1;
    *ny = jsize - 1;

    elev->assign((isize - 1) * (jsize - 1), 0);
    ier = cg_iRIC_Read_Grid_Real_Cell("Elevation", elev->data());
	ier = cg_close(fn);
}

void gen_input(const std::string& dem_fname, int nx, int ny, const std::vector<double>& elev)
{
    std::ofstream f(dem_fname.c_str());

    f << "ncols " << nx << std::endl;
    f << "nrows " << ny << std::endl;
    f << "xllcorner 0" << std::endl;
    f << "yllcorner 0" << std::endl;
    f << "cellsize 1" << std::endl;
    f << "NODATA_value -9999" << std::endl;

    for (int j = 0; j < ny; ++j) {
        for (int i = 0; i < nx; ++i) {
            int idx = i + j * nx;
            f << " " << elev[idx];
            if (i == nx - 1) {
                f << std::endl;
            } else {
                f << ",";
            }
        }
    }
    f.close();
}

void launch_mesh1v3(const std::string& demname, const std::string& aacname, const std::string& dirname)
{
	QStringList args;
	args << demname.c_str() << aacname.c_str() << dirname.c_str() << "10000000" << "2";
	QProcess::execute("mesh1v3.exe", args);
}

void load_result(const std::string& outname, int nx, int ny, std::vector<int>* val)
{
    std::string line;
    std::ifstream f(outname.c_str());
    // skip header
    for (int i = 0; i < 6; ++i) {
        std::getline(f, line);
    }
    // load values
    val->clear();
    int num = nx * ny;
    val->reserve(num);
    for (int i = 0; i < num; ++i) {
        int v;
        f >> v;
        val->push_back(v);
    }
}

void output_attr(const std::string& cgns_fname, const std::string& attname, std::vector<int>& val)
{
    int fn, ier;

    ier = cg_open(cgns_fname.c_str(), CG_MODE_MODIFY, &fn);
    ier = cg_iRIC_Init(fn);

    cgsize_t isize, jsize;
    ier = cg_iRIC_GotoGridCoord2d(&isize, &jsize);
    ier = cg_iRIC_Write_Grid_Integer_Cell(attname.c_str(), val.data());
	ier = cg_close(fn);
}
