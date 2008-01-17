/*----------------------------------------------------------------------
$Id: Group.cc,v 1.28 2007/10/04 22:45:01 wmtan Exp $
----------------------------------------------------------------------*/
#include <string>
#include "DataFormats/Common/interface/BasicHandle.h"
#include "FWCore/Framework/src/Group.h"
#include "FWCore/Utilities/interface/ReflexTools.h"

using ROOT::Reflex::Type;
using ROOT::Reflex::TypeTemplate;

namespace edm {

  Group::Group(std::auto_ptr<Provenance> prov,
	       bool onDemand) :
    product_(),
    provenance_(prov.release()),
    unavailable_(false),
    onDemand_(onDemand) {
    if (onDemand) return;
    if (!provenance_->product().present()) {
      unavailable_ = true;
    } else if (provenance_->branchEntryDescription() != 0) {
      unavailable_ = !provenance_->isPresent();
    }
  }

  Group::Group(ConstBranchDescription const& bd) :
    product_(),
    provenance_(new Provenance(bd)),
    unavailable_(!bd.present()),
    onDemand_(false) {
  }

  Group::Group(std::auto_ptr<EDProduct> edp,
	       std::auto_ptr<Provenance> prov) :
    product_(edp.release()),
    provenance_(prov.release()),
    unavailable_(false),
    onDemand_(false) {
  }

  Group::~Group() {
  }

  bool 
  Group::productUnavailable() const { 
      if (onDemand_) return false;
      if (branchEntryDescription()) {
	unavailable_ = !provenance_->isPresent();
      } else if (product_) {
        unavailable_ = !product_->isPresent();
      }
      return unavailable_;
  }

  bool 
  Group::provenanceAvailable() const { 
      return not onDemand_;
  }

  void 
  Group::setProduct(std::auto_ptr<EDProduct> prod) const {
    if(prod.get() == 0) {
      unavailable_ = false;
    } else {
      assert (product() == 0);
      product_ = boost::shared_ptr<EDProduct>(prod.release());  // Group takes ownership
    }
  }
  
  void 
  Group::setProvenance(std::auto_ptr<BranchEntryDescription> prov) const {
    assert (branchEntryDescription() == 0);
    provenance_->setEvent(boost::shared_ptr<BranchEntryDescription>(prov.release()));  // Group takes ownership
    unavailable_ = provenance_->isPresent();
  }

  void  
  Group::swap(Group& other) {
    std::swap(product_, other.product_);
    std::swap(provenance_, other.provenance_);
    std::swap(unavailable_, other.unavailable_);
    std::swap(onDemand_, other.onDemand_);
  }

  void
  Group::replace(Group& g) {
    this->swap(g);
  }

  Type
  Group::productType() const
  {
    return Type::ByTypeInfo(typeid(*product()));
  }

  bool
  Group::isMatchingSequence(Type const& wantedElementType) const
  {
    Type value_type;
    bool is_sequence = is_sequence_wrapper(productType(), value_type);
        
    // If our product is not a sequence, we can't match...
    if (!is_sequence) return false;

    Type elementType = value_type; // this is not true for RefVector...

    TypeTemplate valueTypeTemplate = value_type.TemplateFamily();

    return 
      is_sequence 
      ? (elementType==wantedElementType || 
	 elementType.HasBase(wantedElementType))
      : false;      
  }

  BasicHandle
  Group::makeBasicHandle() const
  {
    return BasicHandle(product_.get(), provenance_.get());
  }

  void
  Group::write(std::ostream& os) const 
  {
    // This is grossly inadequate. It is also not critical for the
    // first pass.
    os << std::string("Group for product with ID: ")
       << provenance_->productID();
  }

}
