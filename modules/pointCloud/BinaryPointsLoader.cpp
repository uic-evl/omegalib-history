#include "BinaryPointsLoader.h"

#include <osg/Geode>
#include <osg/Point>

#include <limits>

using namespace omega;
using namespace cyclops;

///////////////////////////////////////////////////////////////////////////////
BinaryPointsLoader::BinaryPointsLoader(): ModelLoader("points-binary")
{
}

///////////////////////////////////////////////////////////////////////////////
BinaryPointsLoader::~BinaryPointsLoader()
{
}

///////////////////////////////////////////////////////////////////////////////
bool BinaryPointsLoader::supportsExtension(const String& ext) 
{ 
	if(ext == "xyzb") return true;
	return false; 
}

///////////////////////////////////////////////////////////////////////////////
bool BinaryPointsLoader::load(ModelAsset* model)
{
    osg::ref_ptr<osg::Group> group = new osg::Group();

	bool result = loadFile(model, group);

    // if successful get last child and add to sceneobject
    if(result)
    {
		osg::Geode* points;
	    points = group->getChild(0)->asGeode();
	    
		model->nodes.push_back(points);

	    group->removeChild(0, 1);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
bool BinaryPointsLoader::loadFile(ModelAsset* model, osg::Group * grp)
{
	if(!grp)
	{
		return false;
	}

  	osg::Vec3Array* verticesP = new osg::Vec3Array();
  	osg::Vec4Array* verticesC = new osg::Vec4Array();

	String path;
	if(DataManager::findFile(model->info->path, path))
	{ 
		int numPoints = 0;
		float maxf = numeric_limits<float>::max();
		float minf = -numeric_limits<float>::max();
		Vector4f rgbamin = Vector4f(maxf, maxf, maxf, maxf);
		Vector4f rgbamax = Vector4f(minf, minf, minf, minf);

		readXYZ(path, model->info->options, verticesP, verticesC,
			&numPoints,
			&rgbamin,
			&rgbamax);

  		// create geometry and geodes to hold the data
  		osg::Geode* geode = new osg::Geode();
  		geode->setCullingActive(false);
  		osg::Geometry* nodeGeom = new osg::Geometry();
  		osg::StateSet *state = nodeGeom->getOrCreateStateSet();
		nodeGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0, verticesP->size()));
  		osg::VertexBufferObject* vboP = nodeGeom->getOrCreateVertexBufferObject();
  		vboP->setUsage (GL_STREAM_DRAW);

  		nodeGeom->setUseDisplayList (false);
  		nodeGeom->setUseVertexBufferObjects(true);
  		nodeGeom->setVertexArray(verticesP);
  		nodeGeom->setColorArray(verticesC);
  		nodeGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  		geode->addDrawable(nodeGeom);
  		geode->dirtyBound();
 
		grp->addChild(geode);

		// Save loade results in the model info
		model->info->loaderOutput = ostr("{ 'numPoints': %d, "
			"'minR': %d, 'maxR': %d, "
			"'minG': %d, 'maxG': %d, "
			"'minB': %d, 'maxB': %d, "
			"'minA': %d, 'maxA': %d }", 
			%numPoints
			%rgbamin[0] %rgbamax[0]
			%rgbamin[1] %rgbamax[1]
			%rgbamin[2] %rgbamax[2]
			%rgbamin[3] %rgbamax[3]
			);
		
		omsg(model->info->loaderOutput);
		
		return true;
	}
	return false; 
}

///////////////////////////////////////////////////////////////////////////////
void BinaryPointsLoader::readXYZ(
	const String& filename, const String& options, 
	osg::Vec3Array* points, osg::Vec4Array* colors,
	int* numPoints,
	Vector4f* rgbamin,
	Vector4f* rgbamax)
{
	osg::Vec3f point;
	osg::Vec4f color(1.0f, 1.0f, 1.0f, 1.0f);

	// Default record size = 7 doubles (X,Y,Z,R,G,B,A)
	int numFields = 7;
	size_t recordSize = sizeof(double) * numFields;

	// Check the options to see if we should read a subsection of the file
	// and / or use decimation.
	int readStartP = 0;
	int readLengthP = 0;
	int decimation = 0;

	if(options != "")
	{
		String format;

		libconfig::ArgumentHelper ah;
		ah.newString("format", "only suported format is xyzrgba", format);
		ah.newNamedInt('s', "start", "start", "start record %", readStartP);
		ah.newNamedInt('l', "length", "length", "number of records to read %", readLengthP);
		ah.newNamedInt('d', "decimation", "decimation", "read decimation", decimation);
		ah.process(options.c_str());
	}

	FILE* fin = fopen(filename.c_str(), "rb");

	// How many records are in the file?
	fseek(fin, 0, SEEK_END);
	long endpos = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	int numRecords = endpos / recordSize;
	int readStart = numRecords * readStartP / 100;
	int readLength = numRecords * readLengthP / 100;

	if(decimation <= 0) decimation = 1;
	if(readStart != 0)
	{
		fseek(fin, readStart * recordSize, SEEK_SET);
	}

	// Adjust read length.
	if(readLength == 0 || readStart + readLength > numRecords)
	{
		readLength = numRecords - readStart;
	}

	ofmsg("BinaryPointsLoader: reading records %1% - %2% of %3% (decimation %4%) of %5%",
		%readStart %(readStart + readLength) %numRecords %decimation %filename);
		
	// Read in data
	double* buffer = (double*)malloc(recordSize * readLength);
	if(buffer == NULL)
	{
		oferror("BinaryPointsLoader::readXYZ: could not allocate %1% bytes", 
			%(recordSize * readLength));
		return;
	}

	size_t size = fread(buffer, recordSize, readLength, fin);

	int ne = readLength / decimation;
	points->reserve(ne);
	colors->reserve(ne);

	int j = 0;
	for(int i = 0; i < readLength; i += decimation)
	{
		point[0] = buffer[i * numFields];
		point[1] = buffer[i * numFields + 1];
		point[2] = buffer[i * numFields + 2];

		color[0] = buffer[i * numFields + 3];
		color[1] = buffer[i * numFields + 4];
		color[2] = buffer[i * numFields + 5];
		color[3] = buffer[i * numFields + 6];

		points->push_back(point);
		colors->push_back(color);

		// Update data bounds and number of points
		*numPoints = *numPoints + 1;
		for(int j = 0; j < 4; j++)
		{
			if(color[j] < (*rgbamin)[j]) (*rgbamin)[j] = color[j];
			if(color[j] > (*rgbamax)[j]) (*rgbamax)[j] = color[j];
		}
	}
	
	fclose(fin);
	free(buffer);
}
