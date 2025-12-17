#pragma once

namespace shim {

    namespace bionic {
        template <typename T>
        auto to_host(T const *);

        template <typename T>
        auto from_host(T const *);
    }

    namespace detail {

        template <typename T>
        struct nop_arg_rewrite {
            using source = T;
            T before(T x) {
                return x;
            }
            void after(T) {
            }
        };

        template <typename T>
        struct arg_rewrite : nop_arg_rewrite<T> {};


        template <typename T>
        struct destroy_invoker {
            T t;
            destroy_invoker(T const &t) : t(t) {}
            ~destroy_invoker() { t(); }
        };
        template <typename T>
        destroy_invoker<T> make_destroy_invoker(T const &t) {
            return destroy_invoker<T>(t);
        }

        template <typename ...Args>
        struct arg_rewriter_impl;
        template <typename Arg, typename ...Args>
        struct arg_rewriter_impl<Arg, Args...> {
            template <typename Invoke, typename ...RArgs>
            static auto rewrite(Invoke const &invoke, RArgs... rargs, typename arg_rewrite<Arg>::source arg, typename arg_rewrite<Args>::source... args) {
                arg_rewrite<Arg> rewritten;
                auto value = rewritten.before(arg);
                auto i = make_destroy_invoker ([&rewritten, &arg] { rewritten.after(arg); });
                return arg_rewriter_impl<Args...>::template rewrite<Invoke, RArgs..., decltype(value)>(invoke, rargs..., value, args...);
            }
        };
        template <>
        struct arg_rewriter_impl<> {
            template <typename Invoke, typename ...RArgs>
            static auto rewrite(Invoke const &invoke, RArgs... args) {
                return invoke(args...);
            }
        };

        template <typename T>
        struct arg_rewrite_helper;
        template <typename Ret, typename ...Args>
        struct arg_rewrite_helper<Ret (Args...)> {
            template <Ret (*Ptr)(Args...)>
            static Ret rewrite(typename arg_rewrite<Args>::source... args) {
                return arg_rewriter_impl<Args...>::rewrite(Ptr, args...);
            }
        };
        template <typename Ret, typename ...Args>
        struct arg_rewrite_helper<Ret (Args...) noexcept> : arg_rewrite_helper<Ret (Args...)> {
        };


        template <typename T, typename BionicT>
        struct bionic_ptr_rewriter {
            using source = BionicT;

            T before(source src) {
                return bionic::to_host(src);
            }
            void after(source src) {
            }
        };


        template <typename Resolver, bool IsWrapped = Resolver::is_wrapped>
        struct wrapper_resolved_ptr_rewriter;
        template <typename Resolver>
        struct wrapper_resolved_ptr_rewriter<Resolver, true> : bionic_ptr_rewriter<typename Resolver::host_type *, typename Resolver::wrapper_type *> {};
        template <typename Resolver>
        struct wrapper_resolved_ptr_rewriter<Resolver, false> : nop_arg_rewrite<typename Resolver::host_type *> {};

    }

}


#define AutoArgRewritten(ptr) (&shim::detail::arg_rewrite_helper<decltype(ptr)>::rewrite<ptr>)