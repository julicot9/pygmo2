// Copyright 2019 PaGMO development team
//
// This file is part of the pygmo library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/compass_search.hpp>
#include <pagmo/algorithms/de.hpp>
#include <pagmo/algorithms/mbh.hpp>
#include <pagmo/detail/make_unique.hpp>
#include <pagmo/population.hpp>
#include <pagmo/rng.hpp>
#include <pagmo/threading.hpp>
#include <pagmo/types.hpp>

#include "docstrings.hpp"
#include "expose_algorithms.hpp"

namespace pygmo
{

namespace py = pybind11;

namespace detail
{

namespace
{

// A test algo.
struct test_algorithm {
    pagmo::population evolve(const pagmo::population &pop) const
    {
        return pop;
    }
    // Set/get an internal value to test extraction semantics.
    void set_n(int n)
    {
        m_n = n;
    }
    int get_n() const
    {
        return m_n;
    }
    int m_n = 1;
};

// A thread unsafe test algo.
struct tu_test_algorithm {
    pagmo::population evolve(const pagmo::population &pop) const
    {
        return pop;
    }
    pagmo::thread_safety get_thread_safety() const
    {
        return pagmo::thread_safety::none;
    }
};

} // namespace

} // namespace detail

void expose_algorithms_0(py::module &m, py::class_<pagmo::algorithm> &algo, py::module &a_module)
{
    // Test algo.
    auto test_a = expose_algorithm<detail::test_algorithm>(m, algo, a_module, "_test_algorithm", "A test algorithm.");
    test_a.def("get_n", &detail::test_algorithm::get_n);
    test_a.def("set_n", &detail::test_algorithm::set_n);

    // Thread unsafe test algo.
    expose_algorithm<detail::tu_test_algorithm>(m, algo, a_module, "_tu_test_algorithm",
                                                "A thread unsafe test algorithm.");

    // DE
    auto de_ = expose_algorithm<pagmo::de>(m, algo, a_module, "de", de_docstring().c_str());
    de_.def(py::init<unsigned, double, double, unsigned, double, double>(), py::arg("gen") = 1u, py::arg("F") = .8,
            py::arg("CR") = .9, py::arg("variant") = 2u, py::arg("ftol") = 1e-6, py::arg("xtol") = 1E-6);
    de_.def(py::init<unsigned, double, double, unsigned, double, double, unsigned>(), py::arg("gen") = 1u,
            py::arg("F") = .8, py::arg("CR") = .9, py::arg("variant") = 2u, py::arg("ftol") = 1e-6,
            py::arg("xtol") = 1E-6, py::arg("seed"));
    expose_algo_log(de_, de_get_log_docstring().c_str());
    de_.def("get_seed", &pagmo::de::get_seed, generic_uda_get_seed_docstring().c_str());

    // MBH meta-algo.
    auto mbh_ = expose_algorithm<pagmo::mbh>(m, algo, a_module, "mbh", mbh_docstring().c_str());
    mbh_.def(py::init([](const pagmo::algorithm &a, unsigned stop, const py::array_t<double> &perturb, unsigned seed) {
        return pagmo::detail::make_unique<pagmo::mbh>(a, stop, ndarr_to_vector<pagmo::vector_double>(perturb), seed);
    }));
    mbh_.def(py::init([](const pagmo::algorithm &a, unsigned stop, const py::array_t<double> &perturb) {
        return pagmo::detail::make_unique<pagmo::mbh>(a, stop, ndarr_to_vector<pagmo::vector_double>(perturb),
                                                      pagmo::random_device::next());
    }));
    mbh_.def("get_seed", &pagmo::mbh::get_seed, mbh_get_seed_docstring().c_str());
    mbh_.def("get_verbosity", &pagmo::mbh::get_verbosity, mbh_get_verbosity_docstring().c_str());
    mbh_.def(
        "set_perturb",
        [](pagmo::mbh &a, const py::array_t<double> &o) { a.set_perturb(ndarr_to_vector<pagmo::vector_double>(o)); },
        mbh_set_perturb_docstring().c_str(), py::arg("perturb"));
    expose_algo_log(mbh_, mbh_get_log_docstring().c_str());
    mbh_.def(
        "get_perturb", [](const pagmo::mbh &a) { return vector_to_ndarr<py::array_t<double>>(a.get_perturb()); },
        mbh_get_perturb_docstring().c_str());
    mbh_.def_property_readonly(
        "inner_algorithm", [](pagmo::mbh &uda) -> pagmo::algorithm & { return uda.get_inner_algorithm(); },
        py::return_value_policy::reference_internal, generic_uda_inner_algorithm_docstring().c_str());

    // Compass search.
    auto compass_search_ = expose_algorithm<pagmo::compass_search>(m, algo, a_module, "compass_search",
                                                                   compass_search_docstring().c_str());
    compass_search_.def(py::init<unsigned, double, double, double>(), py::arg("max_fevals") = 1u,
                        py::arg("start_range") = .1, py::arg("stop_range") = .01, py::arg("reduction_coeff") = .5);
    expose_algo_log(compass_search_, compass_search_get_log_docstring().c_str());
    compass_search_.def("get_max_fevals", &pagmo::compass_search::get_max_fevals);
    compass_search_.def("get_start_range", &pagmo::compass_search::get_start_range);
    compass_search_.def("get_stop_range", &pagmo::compass_search::get_stop_range);
    compass_search_.def("get_reduction_coeff", &pagmo::compass_search::get_reduction_coeff);
    compass_search_.def("get_verbosity", &pagmo::compass_search::get_verbosity);
    expose_not_population_based(compass_search_, "compass_search");

#if 0
    // cstrs_self_adaptive meta-algo.
    auto cstrs_sa
        = expose_algorithm_pygmo<cstrs_self_adaptive>("cstrs_self_adaptive", cstrs_self_adaptive_docstring().c_str());
    cstrs_sa.def("__init__", bp::make_constructor(lcast([](unsigned iters, const algorithm &a, unsigned seed) {
                                                      return ::new pagmo::cstrs_self_adaptive(iters, a, seed);
                                                  }),
                                                  bp::default_call_policies()));
    cstrs_sa.def("__init__", bp::make_constructor(lcast([](unsigned iters, const algorithm &a) {
                                                      return ::new pagmo::cstrs_self_adaptive(
                                                          iters, a, pagmo::random_device::next());
                                                  }),
                                                  bp::default_call_policies()));
    expose_algo_log(cstrs_sa, cstrs_self_adaptive_get_log_docstring().c_str());
    add_property(
        cstrs_sa, "inner_algorithm",
        bp::make_function(lcast([](cstrs_self_adaptive &uda) -> algorithm & { return uda.get_inner_algorithm(); }),
                          bp::return_internal_reference<>()),
        generic_uda_inner_algorithm_docstring().c_str());

    // ARTIFICIAL BEE COLONY
    auto bee_colony_ = expose_algorithm_pygmo<bee_colony>("bee_colony", bee_colony_docstring().c_str());
    bee_colony_.def(bp::init<unsigned, unsigned>((bp::arg("gen") = 1u, bp::arg("limit") = 1u)));
    bee_colony_.def(
        bp::init<unsigned, unsigned, unsigned>((bp::arg("gen") = 1u, bp::arg("limit") = 20u, bp::arg("seed"))));
    expose_algo_log(bee_colony_, bee_colony_get_log_docstring().c_str());
    bee_colony_.def("get_seed", &bee_colony::get_seed, generic_uda_get_seed_docstring().c_str());



    // DE-1220
    auto de1220_ = expose_algorithm_pygmo<de1220>("de1220", de1220_docstring().c_str());
    // Helper to get the list of default allowed variants for de1220.
    auto de1220_allowed_variants = []() -> bp::list {
        bp::list retval;
        for (const auto &n : de1220_statics<void>::allowed_variants) {
            retval.append(n);
        }
        return retval;
    };
    de1220_.def("__init__",
                bp::make_constructor(lcast([](unsigned gen, const bp::object &allowed_variants, unsigned variant_adptv,
                                              double ftol, double xtol, bool memory) -> de1220 * {
                                         auto av = obj_to_vector<std::vector<unsigned>>(allowed_variants);
                                         return ::new de1220(gen, av, variant_adptv, ftol, xtol, memory);
                                     }),
                                     bp::default_call_policies(),
                                     (bp::arg("gen") = 1u, bp::arg("allowed_variants") = de1220_allowed_variants(),
                                      bp::arg("variant_adptv") = 1u, bp::arg("ftol") = 1e-6, bp::arg("xtol") = 1e-6,
                                      bp::arg("memory") = false)));
    de1220_.def("__init__",
                bp::make_constructor(lcast([](unsigned gen, const bp::object &allowed_variants, unsigned variant_adptv,
                                              double ftol, double xtol, bool memory, unsigned seed) -> de1220 * {
                                         auto av = obj_to_vector<std::vector<unsigned>>(allowed_variants);
                                         return ::new de1220(gen, av, variant_adptv, ftol, xtol, memory, seed);
                                     }),
                                     bp::default_call_policies(),
                                     (bp::arg("gen") = 1u, bp::arg("allowed_variants") = de1220_allowed_variants(),
                                      bp::arg("variant_adptv") = 1u, bp::arg("ftol") = 1e-6, bp::arg("xtol") = 1e-6,
                                      bp::arg("memory") = false, bp::arg("seed"))));
    expose_algo_log(de1220_, de1220_get_log_docstring().c_str());
    de1220_.def("get_seed", &de1220::get_seed, generic_uda_get_seed_docstring().c_str());

#if defined(PAGMO_WITH_EIGEN3)
    // CMA-ES
    auto cmaes_ = expose_algorithm_pygmo<cmaes>("cmaes", cmaes_docstring().c_str());
    cmaes_.def(bp::init<unsigned, double, double, double, double, double, double, double, bool, bool>(
        (bp::arg("gen") = 1u, bp::arg("cc") = -1., bp::arg("cs") = -1., bp::arg("c1") = -1., bp::arg("cmu") = -1.,
         bp::arg("sigma0") = 0.5, bp::arg("ftol") = 1e-6, bp::arg("xtol") = 1e-6, bp::arg("memory") = false,
         bp::arg("force_bounds") = false)));
    cmaes_.def(bp::init<unsigned, double, double, double, double, double, double, double, bool, bool, unsigned>(
        (bp::arg("gen") = 1u, bp::arg("cc") = -1., bp::arg("cs") = -1., bp::arg("c1") = -1., bp::arg("cmu") = -1.,
         bp::arg("sigma0") = 0.5, bp::arg("ftol") = 1e-6, bp::arg("xtol") = 1e-6, bp::arg("memory") = false,
         bp::arg("force_bounds") = false, bp::arg("seed"))));
    expose_algo_log(cmaes_, cmaes_get_log_docstring().c_str());
    cmaes_.def("get_seed", &cmaes::get_seed, generic_uda_get_seed_docstring().c_str());

    // xNES
    auto xnes_ = expose_algorithm_pygmo<xnes>("xnes", xnes_docstring().c_str());
    xnes_.def(bp::init<unsigned, double, double, double, double, double, double, bool, bool>(
        (bp::arg("gen") = 1u, bp::arg("eta_mu") = -1., bp::arg("eta_sigma") = -1., bp::arg("eta_b") = -1.,
         bp::arg("sigma0") = -1, bp::arg("ftol") = 1e-6, bp::arg("xtol") = 1e-6, bp::arg("memory") = false,
         bp::arg("force_bounds") = false)));
    xnes_.def(bp::init<unsigned, double, double, double, double, double, double, bool, bool, unsigned>(
        (bp::arg("gen") = 1u, bp::arg("eta_mu") = -1., bp::arg("eta_sigma") = -1., bp::arg("eta_b") = -1.,
         bp::arg("sigma0") = -1, bp::arg("ftol") = 1e-6, bp::arg("xtol") = 1e-6, bp::arg("memory") = false,
         bp::arg("force_bounds") = false, bp::arg("seed"))));
    expose_algo_log(xnes_, xnes_get_log_docstring().c_str());
    xnes_.def("get_seed", &xnes::get_seed, generic_uda_get_seed_docstring().c_str());
#endif

    // MOEA/D - DE
    auto moead_ = expose_algorithm_pygmo<moead>("moead", moead_docstring().c_str());
    moead_.def(bp::init<unsigned, std::string, std::string, unsigned, double, double, double, double, unsigned, bool>(
        (bp::arg("gen") = 1u, bp::arg("weight_generation") = "grid", bp::arg("decomposition") = "tchebycheff",
         bp::arg("neighbours") = 20u, bp::arg("CR") = 1., bp::arg("F") = 0.5, bp::arg("eta_m") = 20,
         bp::arg("realb") = 0.9, bp::arg("limit") = 2u, bp::arg("preserve_diversity") = true)));
    moead_.def(bp::init<unsigned, std::string, std::string, unsigned, double, double, double, double, unsigned, bool,
                        unsigned>(
        (bp::arg("gen") = 1u, bp::arg("weight_generation") = "grid", bp::arg("decomposition") = "tchebycheff",
         bp::arg("neighbours") = 20u, bp::arg("CR") = 1., bp::arg("F") = 0.5, bp::arg("eta_m") = 20,
         bp::arg("realb") = 0.9, bp::arg("limit") = 2u, bp::arg("preserve_diversity") = true, bp::arg("seed"))));
    // moead needs an ad hoc exposition for the log as one entry is a vector (ideal_point)
    moead_.def("get_log", lcast([](const moead &a) -> bp::list {
                   bp::list retval;
                   for (const auto &t : a.get_log()) {
                       retval.append(bp::make_tuple(std::get<0>(t), std::get<1>(t), std::get<2>(t),
                                                    vector_to_ndarr(std::get<3>(t))));
                   }
                   return retval;
               }),
               moead_get_log_docstring().c_str());
    moead_.def("get_seed", &moead::get_seed, generic_uda_get_seed_docstring().c_str());

#if defined(PAGMO_WITH_IPOPT)
    // Ipopt.
    auto ipopt_ = expose_algorithm_pygmo<ipopt>("ipopt", ipopt_docstring().c_str());
    expose_not_population_based(ipopt_, "ipopt");
    expose_algo_log(ipopt_, ipopt_get_log_docstring().c_str());
    ipopt_.def("get_last_opt_result", lcast([](const ipopt &ip) { return static_cast<int>(ip.get_last_opt_result()); }),
               ipopt_get_last_opt_result_docstring().c_str());
    // Options management.
    // String opts.
    ipopt_.def("set_string_option", &ipopt::set_string_option, ipopt_set_string_option_docstring().c_str(),
               (bp::arg("name"), bp::arg("value")));
    ipopt_.def("set_string_options", lcast([](ipopt &ip, const bp::dict &d) {
                   std::map<std::string, std::string> m;
                   bp::stl_input_iterator<std::string> begin(d), end;
                   for (; begin != end; ++begin) {
                       m[*begin] = bp::extract<std::string>(d[*begin])();
                   }
                   ip.set_string_options(m);
               }),
               ipopt_set_string_options_docstring().c_str(), bp::arg("opts"));
    ipopt_.def("get_string_options", lcast([](const ipopt &ip) -> bp::dict {
                   const auto opts = ip.get_string_options();
                   bp::dict retval;
                   for (const auto &p : opts) {
                       retval[p.first] = p.second;
                   }
                   return retval;
               }),
               ipopt_get_string_options_docstring().c_str());
    ipopt_.def("reset_string_options", &ipopt::reset_string_options, ipopt_reset_string_options_docstring().c_str());
    // Integer options.
    ipopt_.def("set_integer_option", &ipopt::set_integer_option, ipopt_set_integer_option_docstring().c_str(),
               (bp::arg("name"), bp::arg("value")));
    ipopt_.def("set_integer_options", lcast([](ipopt &ip, const bp::dict &d) {
                   std::map<std::string, Ipopt::Index> m;
                   bp::stl_input_iterator<std::string> begin(d), end;
                   for (; begin != end; ++begin) {
                       m[*begin] = bp::extract<Ipopt::Index>(d[*begin])();
                   }
                   ip.set_integer_options(m);
               }),
               ipopt_set_integer_options_docstring().c_str(), bp::arg("opts"));
    ipopt_.def("get_integer_options", lcast([](const ipopt &ip) -> bp::dict {
                   const auto opts = ip.get_integer_options();
                   bp::dict retval;
                   for (const auto &p : opts) {
                       retval[p.first] = p.second;
                   }
                   return retval;
               }),
               ipopt_get_integer_options_docstring().c_str());
    ipopt_.def("reset_integer_options", &ipopt::reset_integer_options, ipopt_reset_integer_options_docstring().c_str());
    // Numeric options.
    ipopt_.def("set_numeric_option", &ipopt::set_numeric_option, ipopt_set_numeric_option_docstring().c_str(),
               (bp::arg("name"), bp::arg("value")));
    ipopt_.def("set_numeric_options", lcast([](ipopt &ip, const bp::dict &d) {
                   std::map<std::string, double> m;
                   bp::stl_input_iterator<std::string> begin(d), end;
                   for (; begin != end; ++begin) {
                       m[*begin] = bp::extract<double>(d[*begin])();
                   }
                   ip.set_numeric_options(m);
               }),
               ipopt_set_numeric_options_docstring().c_str(), bp::arg("opts"));
    ipopt_.def("get_numeric_options", lcast([](const ipopt &ip) -> bp::dict {
                   const auto opts = ip.get_numeric_options();
                   bp::dict retval;
                   for (const auto &p : opts) {
                       retval[p.first] = p.second;
                   }
                   return retval;
               }),
               ipopt_get_numeric_options_docstring().c_str());
    ipopt_.def("reset_numeric_options", &ipopt::reset_numeric_options, ipopt_reset_numeric_options_docstring().c_str());
#endif
#endif
}

} // namespace pygmo