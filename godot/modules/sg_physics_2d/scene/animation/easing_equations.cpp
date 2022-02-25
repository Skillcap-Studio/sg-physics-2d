/**
 *  Adapted from Penner Easing equations' C++ port.
 *  Source: https://github.com/jesusgollonet/ofpennereasing
 *  License: BSD-3-clause
 */

#include "sg_tween.h"

const fixed pi = fixed(205887);

///////////////////////////////////////////////////////////////////////////
// linear
///////////////////////////////////////////////////////////////////////////
namespace linear {
	static fixed in(fixed t, fixed b, fixed c, fixed d) {
		return c * t / d + b;
	}

	static fixed out(fixed t, fixed b, fixed c, fixed d) {
		return c * t / d + b;
	}

	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
		return c * t / d + b;
	}

	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
		return c * t / d + b;
	}
}; // namespace linear
///////////////////////////////////////////////////////////////////////////
// sine
///////////////////////////////////////////////////////////////////////////
namespace sine {
	static fixed in(fixed t, fixed b, fixed c, fixed d) {
		return -c * (t / d * (pi >> 1)).cos() + c + b;
	}

	static fixed out(fixed t, fixed b, fixed c, fixed d) {
		return c * (t / d * (pi >> 1)).sin() + b;
	}

	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
		return -(c >> 1) * ((pi * t / d).cos() - fixed::ONE) + b;
	}

	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
		return (t < (d >> 1)) ? out(t << 1, b, c >> 1, d) : in((t << 1) - d, b + c >> 1, c >> 1, d);
	}
}; // namespace sine
/////////////////////////////////////////////////////////////////////////////
//// quint
/////////////////////////////////////////////////////////////////////////////
//namespace quint {
//	static fixed in(fixed t, fixed b, fixed c, fixed d) {
//		return c * pow(t / d, 5) + b;
//	}
//
//	static fixed out(fixed t, fixed b, fixed c, fixed d) {
//		return c * (pow(t / d - 1, 5) + 1) + b;
//	}
//
//	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
//		t = t / d * 2;
//		if (t < 1) return c / 2 * pow(t, 5) + b;
//		return c / 2 * (pow(t - 2, 5) + 2) + b;
//	}
//
//	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
//		return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
//	}
//}; // namespace quint
/////////////////////////////////////////////////////////////////////////////
//// quart
/////////////////////////////////////////////////////////////////////////////
//namespace quart {
//	static fixed in(fixed t, fixed b, fixed c, fixed d) {
//		return c * pow(t / d, 4) + b;
//	}
//
//	static fixed out(fixed t, fixed b, fixed c, fixed d) {
//		return -c * (pow(t / d - 1, 4) - 1) + b;
//	}
//
//	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
//		t = t / d * 2;
//		if (t < 1) return c / 2 * pow(t, 4) + b;
//		return -c / 2 * (pow(t - 2, 4) - 2) + b;
//	}
//
//	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
//		return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
//	}
//}; // namespace quart
///////////////////////////////////////////////////////////////////////////
// quad
///////////////////////////////////////////////////////////////////////////
namespace quad {
	static fixed in(fixed t, fixed b, fixed c, fixed d) {
		return c * (t / d) * (t / d) + b;
	}

	static fixed out(fixed t, fixed b, fixed c, fixed d) {
		t = t / d;
		return -c * t * (t - fixed(131072)) + b;
	}

	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
		t = (t / d) << 1;
		if (t < fixed::ONE) return (c >> 1) * t * t + b;
		return -(c >> 1) * ((t - fixed::ONE) * (t - fixed(196608)) - fixed::ONE) + b;
	}

	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
		return (t < (d >> 1)) ? out(t << 1, b, c >> 1, d) : in((t << 1) - d, b + c >> 1, c >> 1, d);
	}
}; // namespace quad
/////////////////////////////////////////////////////////////////////////////
//// expo
/////////////////////////////////////////////////////////////////////////////
//namespace expo {
//	static fixed in(fixed t, fixed b, fixed c, fixed d) {
//		if (t == 0) return b;
//		return c * pow(2, 10 * (t / d - 1)) + b - c * 0.001;
//	}
//
//	static fixed out(fixed t, fixed b, fixed c, fixed d) {
//		if (t == d) return b + c;
//		return c * 1.001 * (-pow(2, -10 * t / d) + 1) + b;
//	}
//
//	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
//		if (t == 0) return b;
//		if (t == d) return b + c;
//		t = t / d * 2;
//		if (t < 1) return c / 2 * pow(2, 10 * (t - 1)) + b - c * 0.0005;
//		return c / 2 * 1.0005 * (-pow(2, -10 * (t - 1)) + 2) + b;
//	}
//
//	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
//		return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
//	}
//}; // namespace expo
/////////////////////////////////////////////////////////////////////////////
//// elastic
/////////////////////////////////////////////////////////////////////////////
//namespace elastic {
//	static fixed in(fixed t, fixed b, fixed c, fixed d) {
//		if (t == 0) return b;
//		if ((t /= d) == 1) return b + c;
//		float p = d * 0.3f;
//		float a = c;
//		float s = p / 4;
//		float postFix = a * pow(2, 10 * (t -= 1)); // this is a fix, again, with post-increment operators
//		return -(postFix * sin((t * d - s) * (2 * pi) / p)) + b;
//	}
//
//	static fixed out(fixed t, fixed b, fixed c, fixed d) {
//		if (t == 0) return b;
//		if ((t /= d) == 1) return b + c;
//		float p = d * 0.3f;
//		float a = c;
//		float s = p / 4;
//		return (a * pow(2, -10 * t) * sin((t * d - s) * (2 * pi) / p) + c + b);
//	}
//
//	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
//		if (t == 0) return b;
//		if ((t /= d / 2) == 2) return b + c;
//		float p = d * (0.3f * 1.5f);
//		float a = c;
//		float s = p / 4;
//
//		if (t < 1) {
//			float postFix = a * pow(2, 10 * (t -= 1)); // postIncrement is evil
//			return -0.5f * (postFix * sin((t * d - s) * (2 * pi) / p)) + b;
//		}
//		float postFix = a * pow(2, -10 * (t -= 1)); // postIncrement is evil
//		return postFix * sin((t * d - s) * (2 * pi) / p) * 0.5f + c + b;
//	}
//
//	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
//		return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
//	}
//}; // namespace elastic
///////////////////////////////////////////////////////////////////////////
// cubic
///////////////////////////////////////////////////////////////////////////
namespace cubic {
	static fixed in(fixed t, fixed b, fixed c, fixed d) {
		t /= d;
		return c * t * t * t + b;
	}

	static fixed out(fixed t, fixed b, fixed c, fixed d) {
		t = t / d - fixed::ONE;
		return c * (t * t * t + fixed::ONE) + b;
	}

	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
		t /= (d >> 1);
		if (t < fixed::ONE) return (c >> 1) * t * t * t + b;
		t -= fixed(131072);
		return (c >> 1) * (t * t * t + fixed(131072)) + b;
	}

	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
		return (t < (d >> 1)) ? out(t << 1, b, c >> 1, d) : in((t << 1) - d, b + (c >> 1), c >> 1, d);
	}
}; // namespace cubic
///////////////////////////////////////////////////////////////////////////
// circ
///////////////////////////////////////////////////////////////////////////
namespace circ {
	static fixed in(fixed t, fixed b, fixed c, fixed d) {
		t /= d;
		return -c * ((fixed::ONE - t * t).sqrt() - fixed::ONE) + b;
	}

	static fixed out(fixed t, fixed b, fixed c, fixed d) {
		t = t / d - fixed::ONE;
		return c * (fixed::ONE - t * t).sqrt() + b;
	}

	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
		t /= (d >> 1);
		if (t < fixed::ONE) {
			return -(c >> 1) * ((fixed::ONE - t * t).sqrt() - fixed::ONE) + b;
		}
		t -= fixed(131072);
		return (c >> 1) * ((fixed::ONE - t * t).sqrt() + fixed::ONE) + b;
	}

	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
		return (t < (d >> 1)) ? out(t << 1, b, c >> 1, d) : in((t << 1) - d, b + (c >> 1), c >> 1, d);
	}
}; // namespace circ
///////////////////////////////////////////////////////////////////////////
// bounce
///////////////////////////////////////////////////////////////////////////
namespace bounce {
	static fixed out(fixed t, fixed b, fixed c, fixed d);

	static fixed in(fixed t, fixed b, fixed c, fixed d) {
		return c - out(d - t, fixed(0), c, d) + b;
	}

	static fixed out(fixed t, fixed b, fixed c, fixed d) {
		if ((t /= d) < fixed(23831)) {
			return c * (fixed(495616) * t * t) + b;
		}
		else if (t < fixed(47662)) {
			fixed postFix = t -= fixed(35746);
			return c * (fixed(495616) * (postFix)*t + fixed(49152)) + b;
		}
		else if (t < fixed(59578)) {
			fixed postFix = t -= fixed(53620);
			return c * (fixed(495616) * (postFix)*t + fixed(61440)) + b;
		}
		else {
			fixed postFix = t -= fixed(62557);
			return c * (fixed(495616) * (postFix)*t + fixed(64512)) + b;
		}
	}

	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
		return (t < (d >> 1)) ? in(t << 1, b, c >> 1, d) : out((t << 1) - d, b + (c >> 1), c >> 1, d);
	}

	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
		return (t < (d >> 1)) ? out(t << 1, b, c >> 1, d) : in((t << 1) - d, b + (c >> 1), c >> 1, d);
	}
}; // namespace bounce
///////////////////////////////////////////////////////////////////////////
// back
///////////////////////////////////////////////////////////////////////////
namespace back {
	static fixed in(fixed t, fixed b, fixed c, fixed d) {
		fixed s(111514);
		fixed postFix = t /= d;
		return c * (postFix)*t * ((s + fixed::ONE) * t - s) + b;
	}

	static fixed out(fixed t, fixed b, fixed c, fixed d) {
		fixed s(111514);
		t = t / d - fixed::ONE;
		return c * (t * t * ((s + fixed::ONE) * t + s) + fixed::ONE) + b;
	}

	static fixed in_out(fixed t, fixed b, fixed c, fixed d) {
		fixed s(170059);
		t /= (d >> 1);
		if (t < fixed::ONE) return (c >> 1) * (t * t * ((s + fixed::ONE) * t - s)) + b;
		t -= fixed(131072);
		return (c >> 1) * (t * t * ((s + fixed::ONE) * t + s) + fixed(131072)) + b;
	}

	static fixed out_in(fixed t, fixed b, fixed c, fixed d) {
		return (t < (d >> 1)) ? out(t << 1, b, c >> 1, d) : in((t << 1) - d, b + (c >> 1), c >> 1, d);
	}
}; // namespace back

SGTween::interpolater SGTween::interpolaters[SGTween::TRANS_COUNT][SGTween::EASE_COUNT] = {
	{ &linear::in, &linear::out, &linear::in_out, &linear::out_in },
	{ &sine::in, &sine::out, &sine::in_out, &sine::out_in },
	//{ &quint::in, &quint::out, &quint::in_out, &quint::out_in },
	//{ &quart::in, &quart::out, &quart::in_out, &quart::out_in },
	{ &quad::in, &quad::out, &quad::in_out, &quad::out_in },
	//{ &expo::in, &expo::out, &expo::in_out, &expo::out_in },
	//{ &elastic::in, &elastic::out, &elastic::in_out, &elastic::out_in },
	{ &cubic::in, &cubic::out, &cubic::in_out, &cubic::out_in },
	{ &circ::in, &circ::out, &circ::in_out, &circ::out_in },
	{ &bounce::in, &bounce::out, &bounce::in_out, &bounce::out_in },
	{ &back::in, &back::out, &back::in_out, &back::out_in },
};

fixed SGTween::_run_equation(TransitionType p_trans_type, EaseType p_ease_type, fixed t, fixed b, fixed c, fixed d) {
	if (d == fixed(0)) {
		// Special case to avoid dividing by 0 in equations.
		return b + c;
	}

	interpolater cb = interpolaters[p_trans_type][p_ease_type];
	ERR_FAIL_COND_V(cb == NULL, b);
	return cb(t, b, c, d);
}
