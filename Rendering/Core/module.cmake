vtk_module(vtkRenderingCore
  GROUPS
    Rendering
  DEPENDS
    vtkCommonExecutionModel
    vtkCommonTransforms
    vtkFiltersSources
    vtkFiltersGeneral
    vtkFiltersGeometry
    vtkFiltersExtraction
    vtkIOImage
    vtkIOXMLParser
  COMPILE_DEPENDS
    vtkUtilitiesMaterialLibrary
  TEST_DEPENDS
    vtkIOXML
    vtkTestingCore
    vtkTestingRendering
    vtkRenderingContext2D
  )
