#include "vtkSmartPointer.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkDataSetMapper.h"
#include "vtkExtractSelection.h"
#include "vtkInformation.h"
#include "vtkLinearExtractor.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPlaneSource.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProgrammableFilter.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkBrokenLineWidget.h"
#include "vtkTextProperty.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"

#include "vtkTestUtilities.h"

// Callback for the broken line widget interaction
class vtkBLWCallback : public vtkCommand
{
public:
  static vtkBLWCallback *New()
  { return new vtkBLWCallback; }
  virtual void Execute( vtkObject *caller, unsigned long, void* )
  {
    // Retrieve polydata line
    vtkBrokenLineWidget *line = reinterpret_cast<vtkBrokenLineWidget*>( caller );
    line->GetPolyData( Poly );

    // Update linear extractor with current points
    this->Selector->SetPoints( Poly->GetPoints() );

    // Update selection from mesh
    this->Extractor->Update();
    vtkMultiBlockDataSet* outMB = vtkMultiBlockDataSet::SafeDownCast( this->Extractor->GetOutput() );
    vtkUnstructuredGrid* selection = vtkUnstructuredGrid::SafeDownCast( outMB->GetBlock( 0 ) );
    this->Mapper->SetInput( selection );
  }
vtkBLWCallback():Poly(0),Selector(0),Extractor(0),Mapper(0) {};
  vtkPolyData* Poly;
  vtkLinearExtractor* Selector; 
  vtkExtractSelection* Extractor;
  vtkDataSetMapper* Mapper;
};

int TestBrokenLineWidget( int argc, char *argv[] )
{
  // Initialize test value
  int testIntValue = 0;
   
  // Create render window and interactor
  vtkSmartPointer<vtkRenderWindow> win = vtkSmartPointer<vtkRenderWindow>::New();
  win->SetMultiSamples( 0 );
  win->SetSize( 600, 300 );
  vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow( win );
  iren->Initialize();

  // Create 2 viewports in window
  vtkSmartPointer<vtkRenderer> ren1 = vtkSmartPointer<vtkRenderer>::New();
  ren1->SetBackground( .4, .4, .4 );
  ren1->SetBackground2( .8, .8, .8 );
  ren1->GradientBackgroundOn();
  ren1->SetViewport( 0., 0., .5, 1. );
  win->AddRenderer( ren1 );
  vtkSmartPointer<vtkRenderer> ren2 = vtkSmartPointer<vtkRenderer>::New();
  ren2->SetBackground( 1., 1., 1. );
  ren2->SetViewport( .5, 0., 1., 1. );
  win->AddRenderer( ren2 );

  // Create a good view angle
  vtkCamera* camera1 = ren1->GetActiveCamera();
  camera1->SetFocalPoint( .12, 0., 0. );
  camera1->SetPosition( .35, .3, .3 );
  camera1->SetViewUp( 0., 0., 1. );
  ren2->SetActiveCamera( camera1 );

  // Read 3D unstructured input mesh
  char* fileName = vtkTestUtilities::ExpandDataFileName( argc, argv, "Data/AngularSector.vtk");
  vtkSmartPointer<vtkUnstructuredGridReader> reader 
    = vtkSmartPointer<vtkUnstructuredGridReader>::New();
  reader->SetFileName( fileName );
  delete [] fileName;
  reader->Update();
   
  // Create mesh actor from reader output
  vtkSmartPointer<vtkDataSetMapper> meshMapper = vtkSmartPointer<vtkDataSetMapper>::New();
  meshMapper->SetInputConnection( reader->GetOutputPort() );
  vtkSmartPointer<vtkActor> meshActor = vtkSmartPointer<vtkActor>::New();
  meshActor->SetMapper( meshMapper );
  meshActor->GetProperty()->SetColor( .23, .37, .17 );
  meshActor->GetProperty()->SetRepresentationToWireframe();
  ren1->AddActor( meshActor );

  // Create multi-block mesh for linear extractor
  reader->Update();
  vtkUnstructuredGrid* mesh = reader->GetOutput();
  vtkSmartPointer<vtkMultiBlockDataSet> meshMB = vtkSmartPointer<vtkMultiBlockDataSet>::New();
  meshMB->SetNumberOfBlocks( 1 );
  meshMB->GetMetaData( static_cast<unsigned>( 0 ) )->Set( vtkCompositeDataSet::NAME(), "Mesh" ); 
  meshMB->SetBlock( 0, mesh );

  // Create broken line widget, attach it to input mesh
  vtkSmartPointer<vtkBrokenLineWidget> line = vtkSmartPointer<vtkBrokenLineWidget>::New();
  line->SetInteractor( iren );
  line->SetInput( mesh );
  line->SetPriority( 1. );
  line->KeyPressActivationOff();
  line->PlaceWidget();
  line->ProjectToPlaneOff();
  line->On();
  line->SetResolution( 6 );
  line->SetHandleSizeFactor( 1.5 );

  // Create list of points to define broken line
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint( .23, .0, .0 );
  points->InsertNextPoint( .0, .0, .0 );
  points->InsertNextPoint( .23, .04, .04 );
  line->InitializeHandles( points );

  // Extract polygonal line and then its points
  vtkSmartPointer<vtkPolyData> linePD = vtkSmartPointer<vtkPolyData>::New();
  line->GetPolyData( linePD );
  vtkSmartPointer<vtkPolyDataMapper> lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  lineMapper->SetInput( linePD );
  vtkSmartPointer<vtkActor> lineActor = vtkSmartPointer<vtkActor>::New();
  lineActor->SetMapper( lineMapper );
  lineActor->GetProperty()->SetColor( 1., 0., 0. );
  lineActor->GetProperty()->SetLineWidth( 2. );
  ren2->AddActor( lineActor );

  // Create selection along broken line defined by list of points
  vtkSmartPointer<vtkLinearExtractor> selector = vtkSmartPointer<vtkLinearExtractor>::New();
  selector->SetInput( meshMB );
  selector->SetPoints( points );
  selector->IncludeVerticesOff();
  selector->SetVertexEliminationTolerance( 1.e-12 );

  // Extract selection from mesh
  vtkSmartPointer<vtkExtractSelection> extractor = vtkSmartPointer<vtkExtractSelection>::New();
  extractor->SetInput( 0, meshMB );
  extractor->SetInputConnection( 1, selector->GetOutputPort() );
  extractor->Update();
  vtkMultiBlockDataSet* outMB = vtkMultiBlockDataSet::SafeDownCast( extractor->GetOutput() );
  vtkUnstructuredGrid* selection = vtkUnstructuredGrid::SafeDownCast( outMB->GetBlock( 0 ) );

  // Create selection actor
  vtkSmartPointer<vtkDataSetMapper> selMapper = vtkSmartPointer<vtkDataSetMapper>::New();
  selMapper->SetInput( selection );
  vtkSmartPointer<vtkActor> selActor = vtkSmartPointer<vtkActor>::New();
  selActor->SetMapper( selMapper );
  selActor->GetProperty()->SetColor( 0., 0., 0. );
  selActor->GetProperty()->SetRepresentationToWireframe();
  ren2->AddActor( selActor );

  // Invoke callback on polygonal line to interactively select elements
  vtkSmartPointer<vtkBLWCallback> cb = vtkSmartPointer<vtkBLWCallback>::New();
  cb->Poly = linePD;
  cb->Selector = selector;
  cb->Extractor = extractor;
  cb->Mapper = selMapper;
  line->AddObserver( vtkCommand::InteractionEvent, cb );

  // Render and interact
  win->Render();
  iren->Start();

  return testIntValue;
}
