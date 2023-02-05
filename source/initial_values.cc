//
// SPDX-License-Identifier: MIT
// Copyright (C) 2020 - 2022 by the ryujin authors
//

#include "initial_values.template.h"
#include "euler/description.h" // FIXME refactoring

namespace ryujin
{
  /* instantiations */
  template class InitialValues<Euler::Description, DIM, NUMBER>;

} /* namespace ryujin */
