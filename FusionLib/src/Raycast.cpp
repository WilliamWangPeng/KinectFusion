//
// Created by pbo on 08.07.19.
//

#include "Raycast.hpp"

bool Raycast::surfacePrediction(std::shared_ptr<Frame> currentFrame,std::shared_ptr<Volume> volume,float truncationDistance){

    //TODO: implement raycasting

    auto volumeSize =volume->getVolumeSize();
    auto voxelScale = volume->getVoxelScale();
    auto pose = currentFrame->getGlobalPose();
    Eigen::MatrixXd rotation = pose.block(0,0, 3,3);
    Eigen::MatrixXd translation = pose.block(0,3,3,1);
    auto width = currentFrame->getWidth();
    auto height = currentFrame->getHeight();


    const Eigen::Vector3d volumeRange(volumeSize.x()*voxelScale,volumeSize.y()*voxelScale,volumeSize.z()*voxelScale);

        for( int y =0;y<height;y++){
            for(int x=0;x< width;x++){

                //calculate Normalized Direction
                auto direction = calculateRayDirection(x,y,rotation,currentFrame->getIntrinsics());

                //calculate rayLength
                double parameter(0);
                if(!calcuateEntryParameter(parameter, volumeRange, translation, direction))continue;

                Eigen::Vector3d currentPoint;
//                calculateCurrentPointOnRay(currentPoint,parameter,volumeSize,translation,direction);
                currentPoint = (translation + (direction * parameter)) / voxelScale;


                double TSDF = getTSDF(volume,currentPoint);

                const double maxSearchLength = parameter + volumeRange.x() * sqrt(2.f);
                //why truncationDistance*0.5?

                for(parameter;parameter<maxSearchLength;parameter+=truncationDistance*0.5f){

//                    if(!calculateCurrentPointOnRay(currentPoint,parameter,volumeSize,translation,direction))continue;
                    currentPoint = ((translation + (direction * (parameter + truncationDistance * 0.5f))) / voxelScale);

                    if (currentPoint.x() < 1 || currentPoint.x() >= volumeSize.x() - 1 || currentPoint.y() < 1 ||
                            currentPoint.y() >= volumeSize.y() - 1 ||
                            currentPoint.z() < 1 || currentPoint.z() >= volumeSize.z() - 1)
                        continue;


                    const double previousTSDF = TSDF;
                    TSDF = getTSDF(volume,currentPoint);

                    //This equals -ve to +ve in the paper / we cant go from a negative to positive tsdf value as negative is behind the surface
                    if(previousTSDF<0. && TSDF>0.)break;
                    //this equals +ve to -ve in the paper / this means we just crossed a zero value
                    if(previousTSDF>0. && TSDF<0.){
                        //We reached the zero crossing

                        //TODO: Calculate vertex

                        //TODO: Calculated normal using interpolation method

                        //TODO: set global vertex & normal into currentFrame



                    }



                }





            }
        }



    return true;
}

double Raycast::interpolateNormals(const Eigen::Vector3d, std::shared_ptr<Volume> volume) {
     // TODO implement, check Method signature
    return 0;
}

double get_max_time(const Eigen::Vector3d volumeRange, const Eigen::Vector3d &origin, const Eigen::Vector3d &direction)
{
    double txmax = ((direction.x() > 0 ? volumeRange.x() : 0.f) - origin.x()) / direction.x();
    double tymax = ((direction.y() > 0 ? volumeRange.y(): 0.f) - origin.y()) / direction.y();
    double tzmax = ((direction.z() > 0 ? volumeRange.z() : 0.f) - origin.z()) / direction.z();

    return std::min(std::min(txmax, tymax), tzmax);
}

bool Raycast::calcuateEntryParameter(double &entryParameter, const Eigen::Vector3d volumeRange,
                                     const Eigen::Vector3d &origin,
                                     const Eigen::Vector3d &direction) {
    // TODO implement, check Method signature
    double txmin = ((direction.x() > 0 ? 0.f : volumeRange.x()) - origin.x()) / direction.x();
    double tymin = ((direction.y() > 0 ? 0.f : volumeRange.y()) - origin.y()) / direction.y();
    double tzmin = ((direction.z() > 0 ? 0.f : volumeRange.z()) - origin.z()) / direction.z();

    entryParameter =  std::max(std::max(std::max(txmin, tymin), tzmin),0.);

    if(entryParameter >= get_max_time(volumeRange, origin, direction))
    {
        return false;
    }
    return  true;

}

const Eigen::Vector3d
Raycast::calculateRayDirection(int x, int y, Eigen::Matrix<double, 3, 3, Eigen::DontAlign> rotation,
                               Eigen::Matrix3d intrinsics) {
    // TODO implement, check Method signature
    double fovX = intrinsics(0, 0);
    double fovY = intrinsics(1, 1);
    double cX = intrinsics(0, 2);
    double cY = intrinsics(1, 2);

    Eigen::Vector3d cameraPoint = Eigen::Vector3d((x - cX) / fovX , (y - cY) / fovY ,1.);
    Eigen::Vector3d rayDirection = rotation * cameraPoint;
    rayDirection.normalize();

    return rayDirection;
}

bool Raycast::calculateCurrentPointOnRay(Eigen::Vector3d &currentPoint, double &rayParameter,
                                         const Eigen::Vector3d volumeSize, const Eigen::Vector3d &origin,
                                         const Eigen::Vector3d &direction) {
    // TODO implement, check Method signature
   /* rayParameter += voxelScale;
    currentPoint = (origin + (direction * rayParameter)) / voxelScale;

    if (currentPoint.x() < 1 || currentPoint.x() >= volumeSize.x - 1 || currentPoint.y() < 1 ||
            currentPoint.y() >= volumeSize.y - 1 ||
            currentPoint.z() < 1 || currentPoint.z() >= volumeSize.z - 1)
        return  false;*/

    return true;
}

double Raycast::getTSDF(std::shared_ptr<Volume> volume, Eigen::Vector3d position) {
    auto volumeSize = volume->getVolumeSize();
    // TODO implement, check Method signature
    std::pair<double,double> fusionPoints= volume->getPoints()[position.x() + position.y()*volumeSize.x()
                                                               + position.z()*volumeSize.x()*volumeSize.y()];
    const double tsdf = fusionPoints.first;
    return tsdf;
}
