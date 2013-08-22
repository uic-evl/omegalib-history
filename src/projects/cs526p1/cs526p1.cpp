#include <osgUtil/Optimizer>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Material>

#define OMEGA_NO_GL_HEADERS
#include <omega.h>
#include <omegaToolkit.h>
#include <omegaOsg.h>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgGA/GUIEventHandler>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osgwTools/Shapes.h>
#include <osgwTools/Version.h>
#include <osg/ShapeDrawable>

#include <osgbDynamics/MotionState.h>
#include <osgbCollision/CollisionShapes.h>
#include <osgbDynamics/RigidBody.h>
#include <osgbCollision/Utils.h>

#include <btBulletDynamicsCommon.h>


#include <string>

#include <osg/io_utils>
#include <iostream>

using namespace omega;
using namespace omegaToolkit;
using namespace omegaOsg;
using namespace cyclops;

class ProjectOne: public EngineModule
{
public:
	ProjectOne()
	{
		mySceneManager( NULL ) {}
	}

	virtual void initialize();
	virtual void update(const UpdateContext& context);

private:
	SceneManager* mySceneManager;
};

void ProjectOne::initialize()
{
    osg::Group* root = new osg::Group;

	die1 = makeDie( myWorld );
	die2 = makeDie( myWorld );
    //root->addChild( makeDie( myWorld ) );
    //root->addChild( makeDie( myWorld ) );

	root->addChild(die1);
	root->addChild(die2);
	for (int i=0;i<2;i++)
	{
		printf("%d: (%d, %d, %d)\n", i+1, 
		root->getChild(i)->asTransform()->asMatrixTransform()->getMatrix().getTrans().x(),
		root->getChild(i)->asTransform()->asMatrixTransform()->getMatrix().getTrans().y(),
		root->getChild(i)->asTransform()->asMatrixTransform()->getMatrix().getTrans().z());
	}

    /* BEGIN: Create environment boxes */
    float xDim( 10. );
    float yDim( 10. );
    float zDim( 6. );
    float thick( .1 );

    myShakeBox = new osg::MatrixTransform;
    btCompoundShape* compoundShape = new btCompoundShape;
    { // floor -Z (far back of the shake cube)
        osg::Vec3 halfLengths( xDim*.5, yDim*.5, thick*.5 );
        osg::Vec3 center( 0., 0., -zDim*.5 );
        myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    { // top +Z (invisible, to allow user to see through; no OSG analogue
        osg::Vec3 halfLengths( xDim*.5, yDim*.5, thick*.5 );
        osg::Vec3 center( 0., 0., zDim*.5 );
        //myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    { // left -X
        osg::Vec3 halfLengths( thick*.5, yDim*.5, zDim*.5 );
        osg::Vec3 center( -xDim*.5, 0., 0. );
        myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    { // right +X
        osg::Vec3 halfLengths( thick*.5, yDim*.5, zDim*.5 );
        osg::Vec3 center( xDim*.5, 0., 0. );
        myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    { // bottom of window -Y
        osg::Vec3 halfLengths( xDim*.5, thick*.5, zDim*.5 );
        osg::Vec3 center( 0., -yDim*.5, 0. );
        myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    { // top of window Y
        osg::Vec3 halfLengths( xDim*.5, thick*.5, zDim*.5 );
        osg::Vec3 center( 0., yDim*.5, 0. );
        myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    /* END: Create environment boxes */

    myShakeMotion = new osgbDynamics::MotionState();
    myShakeMotion->setTransform( myShakeBox );
	//myShakeMotion->setWorldTransform( myShakeBox );
    btScalar mass( 0.0 );
    btVector3 inertia( 0, 0, 0 );
    btRigidBody::btRigidBodyConstructionInfo rb( mass, myShakeMotion, compoundShape, inertia );
    myShakeBody = new btRigidBody( rb );
    myShakeBody->setCollisionFlags( myShakeBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT );
    myShakeBody->setActivationState( DISABLE_DEACTIVATION );
    myWorld->addRigidBody( myShakeBody );

	//printf("=====\n=====\nnum of collision objects: %d\n=====\n=====\n",myWorld->getNumCollisionObjects());

    // Create an omegalib scene node and attach the osg node to it. This is used to interact with the 
    // osg object through omegalib interactors.
    myOso = new OsgSceneObject(myShakeBox);
	root->addChild( myOso->getTransformedNode() );
    mySceneNode = new SceneNode(getEngine());
    mySceneNode->addComponent(myOso);
    mySceneNode->setBoundingBoxVisible(true);
    //mySceneNode->setBoundingBoxVisible(false);
    getEngine()->getScene()->addChild(mySceneNode);
	getEngine()->getDefaultCamera()->setPosition(0,0,30);

    // Set the interactor style used to manipulate meshes.
    if(SystemManager::settingExists("config/interactor"))
    {
        Setting& sinteractor = SystemManager::settingLookup("config/interactor");
        myInteractor = ToolkitUtils::createInteractor(sinteractor);
        if(myInteractor != NULL)
        {
            ModuleServices::addModule(myInteractor);
        }
    }

    if(myInteractor != NULL)
    {
        myInteractor->setSceneNode(mySceneNode);
    }
}

void OsgbDice::update(const UpdateContext& context)
{
	printf("Scene Node: (%lf, %lf, %lf)\n", mySceneNode->getPosition().x(), mySceneNode->getPosition().y(), mySceneNode->getPosition().z());
	
	osg::MatrixTransform* shakeTrans = myShakeMotion->getTransform()->asMatrixTransform();
	btVector3 btTrans(shakeTrans->getMatrix().getTrans().x(), shakeTrans->getMatrix().getTrans().y(), -0.25);

	btTransform world;
    myShakeMotion->getWorldTransform( world );
	//btVector3 o = world.getOrigin();
	//o[ 2 ] = -0.25;
	//world.setOrigin( o );
	world.setOrigin( btTrans );
    myShakeMotion->setWorldTransform( world );

	double elapsed = 1./60.;
	myWorld->stepSimulation( elapsed, 4, elapsed/4. );
	//myWorld->stepSimulation( 1./60., 1, 1./60. );
	//prevSimTime = currSimTime;
}

/** \page diceexample The Mandatory Dice Example
No physics-based project would be complete without a dice example. Use the
left mouse button to chake the dice shaker.
*/
int main(int argc, char** argv)
{
	Application<OsgbDice> app("osgbDice");
    return omain(app, argc, argv);
}