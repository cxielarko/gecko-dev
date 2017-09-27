function MakeBigInt(s, l, a)
{
    let a32 = new Int32Array(TypedArrayBuffer(a));
    if (l !== 0) {
        for (let i = l - 1; i >= 0; i--) {
            if (a32[i] === 0) {
                l--;
            } else {
                break;
            }
        }
    }
    if (l === 0)
        s = 0;
    return NewBigInt(s, l, TypedArrayBuffer(a));
}

function BigIntIsZero(x)
{
    return BigIntSize(x) === 0;
}

function MakeSingleDigitBigInt(x) {
    let a = new Uint32Array(1);
    a[0] = x;
    return MakeBigInt(0, 1, a);
}

function MakeZeroBigInt() {
    return NewBigInt(0, 0, undefined);
}

function MakeOneBigInt() {
    return MakeSingleDigitBigInt(1);
}

function BigIntEqual(x, y)
{
    if (BigIntSign(x) !== BigIntSign(y)) {
        return false;
    } else if (BigIntSize(x) !== BigIntSize(y)) {
        return false;
    } else {
        for (let i = 0; i < BigIntSize(x); i++) {
            if (BigIntDigit(x, i) !== BigIntDigit(y, i)) {
                return false;
            }
        }
        return true;
    }
}

function BigIntCompare(x, y)
{
    if (BigIntSign(x)) {
        if (BigIntSign(y)) {
            return -(BigIntCompareUnsigned(x, y));
        } else {
            return -1;
        }
    } else {
        if (BigIntSign(y)) {
            return 1;
        } else {
            if (BigIntSize(x) === BigIntSize(y)) {
                return BigIntCompareUnsigned(x, y);
            } else if (BigIntSize(x) < BigIntSize(y)) {
                return -1;
            } else {
                return 1;
            }
        }
    }
}

function BigIntCompareUnsigned(x, y)
{
    if (BigIntSize(x) === BigIntSize(y)) {
        for (let i = BigIntSize(x) - 1; i >= 0; i--) {
            let xi = BigIntDigit(x, i);
            let yi = BigIntDigit(y, i);
            if (xi < yi) {
                return -1;
            } else if (xi > yi) {
                return 1;
            }
        }
        return 0;
    } else if (BigIntSize(x) < BigIntSize(y)) {
        return -1;
    } else {
        return 1;
    }
}

function BigIntCompareNumber(x, y)
{
    if (typeof x === "bigint" && typeof y === "number") {
        if (Number_isFinite(y)) {
            if (y === 0) {
                return BigIntCompare(x, MakeZeroBigInt());
            }
            let bpair = NumberToRational(y);
            return BigIntCompare(x * bpair[1], bpair[0]);
        } else if (Number_isNaN(y)) {
            return 2;
        } else {
            return (y < 0) ? 1 : -1;
        }
    } else if (typeof x === "number" && typeof y === "bigint") {
        return -BigIntCompareNumber(y, x);
    } else {
        return 0;
    }
}

function BigIntCompareString(x, y)
{
    if (typeof x === "bigint" && typeof y === "string") {
        try {
            if (y === "")
                return BigIntIsZero(x) ? 0 : 1;
            return (x === BigInt_parseInt(y)) ? 0 : 1;
        } catch (e) {
            return 1;
        }
    } else if (typeof x === "string" && typeof y === "bigint") {
        return BigIntCompareString(y, x);
    } else {
        return 1;
    }
}

function BigIntNeg(x)
{
    let arr = new Uint32Array(BigIntSize(x));
    for (let i = 0; i < BigIntSize(x); i++) {
        arr[i] = BigIntDigit(x, i);
    }
    return MakeBigInt(BigIntSign(x) ^ 1, BigIntSize(x), arr);
}

function BigIntComplement(x)
{
    let arr = new Int32Array(BigIntSize(x));
    for (let i = 0; i < BigIntSize(x); i++) {
        arr[i] = ~BigIntDigit(x, i);
    }
    return MakeBigInt(BigIntSign(x), BigIntSize(x), arr);
}

function BigIntAdd(x, y)
{
    if (BigIntIsZero(x)) {
        return y;
    } else if (BigIntIsZero(y)) {
        return x;
    } else if (BigIntSign(x)) {
        if (BigIntSign(y)) {
            return BigIntSumUnsigned(x, y, 1);
        } else {
            return BigIntSubUnsigned(y, x);
        }
    } else {
        if (BigIntSign(y)) {
            return BigIntSubUnsigned(x, y);
        } else {
            return BigIntSumUnsigned(x, y);
        }
    }
}

function BigIntSub(x, y)
{
    if (BigIntIsZero(x)) {
        return BigIntNeg(y);
    } else if (BigIntIsZero(y)) {
        return x;
    } else if (BigIntSign(x)) {
        if (BigIntSign(y)) {
            return BigIntSubUnsigned(y, x);
        } else {
            return BigIntSumUnsigned(x, y, 1);
        }
    } else {
        if (BigIntSign(y)) {
            return BigIntSumUnsigned(x, y);
        } else {
            return BigIntSubUnsigned(x, y);
        }
    }
}

function BigIntMul(x, y)
{
    if (BigIntIsZero(x)) {
        return x;
    } else if (BigIntIsZero(y)) {
        return y;
    } else if (BigIntSign(x)) {
        if (BigIntSign(y)) {
            return BigIntMulUnsigned(x, y);
        } else {
            return BigIntMulUnsigned(x, y, 1);
        }
    } else {
        if (BigIntSign(y)) {
            return BigIntMulUnsigned(x, y, 1);
        } else {
            return BigIntMulUnsigned(x, y);
        }
    }
}

function BigIntDiv(x, y)
{
    if (BigIntIsZero(y)) {
        ThrowRangeError(JSMSG_INTEGER_DIVISION_BY_ZERO);
    } else if (BigIntIsZero(x)) {
        return x;
    } else if (BigIntSign(x)) {
        if (BigIntSign(y)) {
            return BigIntDivUnsigned(x, y);
        } else {
            return BigIntDivUnsigned(x, y, 1);
        }
    } else {
        if (BigIntSign(y)) {
            return BigIntDivUnsigned(x, y, 1);
        } else {
            return BigIntDivUnsigned(x, y);
        }
    }
}

function BigIntMod(x, y)
{
    if (BigIntIsZero(y)) {
        ThrowRangeError(JSMSG_INTEGER_DIVISION_BY_ZERO);
    } else if (BigIntIsZero(x)) {
        return x;
    } else if (BigIntSign(x)) {
        return BigIntModUnsigned(x, y, 1);
    } else {
        return BigIntModUnsigned(x, y);
    }
}

function BigIntPow(x, y)
{
    if (BigIntSign(y)) {
        ThrowRangeError(JSMSG_INTEGER_NEGATIVE_EXPONENT);
    } else {
        return BigIntPowUnsigned(x, y);
    }
}

function BigIntLsh(x, y)
{
    let two = MakeSingleDigitBigInt(2);
    return BigIntMul(x, BigIntPow(two, y));
}

function BigIntRsh(x, y)
{
    let two = MakeSingleDigitBigInt(2);
    let c = BigIntDiv(x, BigIntPow(two, y));
    if (BigIntIsZero(c) && BigIntSign(x)) {
        return BigIntNeg(MakeOneBigInt());
    }
    return c;
}

function BigIntTwosComplement(x)
{
    if (BigIntSign(x)) {
        return BigIntComplement(BigIntNeg(BigIntSubUnsigned(x, MakeOneBigInt())));
    } else {
        return x;
    }
}

function BigIntFromTwosComplement(x)
{
    if (BigIntSign(x)) {
        return BigIntSumUnsigned(BigIntComplement(x), MakeOneBigInt(), BigIntSign(x));
    } else {
        return x;
    }
}

function BigIntTcDigit(x, i)
{
    if (i < BigIntSize(x)) {
        return BigIntDigit(x, i);
    } else if (BigIntSign(x)) {
        return ~0;
    } else {
        return 0;
    }
}

function BigIntBitOp(x, y, fn)
{
    let a2 = BigIntTwosComplement(x);
    let b2 = BigIntTwosComplement(y);
    let ms = std_Math_max(BigIntSize(x), BigIntSize(y));
    let arr = new Int32Array(ms);
    for (let i = 0; i < ms; i++) {
        arr[i] = fn(BigIntTcDigit(a2, i), BigIntTcDigit(b2, i));
    }
    return BigIntFromTwosComplement(MakeBigInt(fn(BigIntSign(x), BigIntSign(y)) ? 1 : 0, ms, arr));
}

function BigIntBitAnd(x, y)
{
    return BigIntBitOp(x, y, (x, y) => x & y);
}

function BigIntBitOr(x, y)
{
    return BigIntBitOp(x, y, (x, y) => x | y);
}

function BigIntBitXor(x, y)
{
    return BigIntBitOp(x, y, (x, y) => x ^ y);
}

function BigIntBitNot(x)
{
    return BigIntFromTwosComplement(BigIntNeg(BigIntComplement(BigIntTwosComplement(x))));
}

#define UINT32(x) ((x) >>> 0)
#define INT32(x) ((x) | 0)
#define BIGINT_ADIGIT_BASE (2**32)
#define BIGINT_ADIGIT(x) (UINT32((x) % BIGINT_ADIGIT_BASE))
#define BIGINT_ADIGIT_SHIFT_DOWN(x) (UINT32((x) / BIGINT_ADIGIT_BASE))
#define BIGINT_MDIGIT_BASE (2**16)
#define BIGINT_MDIGIT(x) (INT32((x) % BIGINT_MDIGIT_BASE))
#define BIGINT_MDIGIT_SHIFT_DOWN(x) (INT32((x) / BIGINT_MDIGIT_BASE))
#define BIGINT_DDIGIT_BASE (2**16)
#define BIGINT_DDIGIT_BITS 16

function BigIntHalfDigit(y, i)
{
    let d = BigIntDigit(y, i >> 1);
    return (i & 1) ? BIGINT_MDIGIT_SHIFT_DOWN(d) : BIGINT_MDIGIT(d);
}

function BigInt_adigit_add_into(c, x, y, dst, idx)
{
    let d = x + y + c;
    dst[idx] = BIGINT_ADIGIT(d);
    return BIGINT_ADIGIT_SHIFT_DOWN(d);
}

function BigInt_adigit_sub_into(c, x, y, dst, idx)
{
    let d = x - y - (c < 0 ? 1 : 0);
    dst[idx] = d;
    return d;
}

function BigInt_mdigit_multiply_add_into(c, x, y1, y2, dst, idx)
{
    let d = UINT32(y1 * y2 + x + c);
    dst[idx] = BIGINT_MDIGIT(d);
    return BIGINT_MDIGIT_SHIFT_DOWN(d);
}

function BigInt_ddigit_divide(divisor, dividend_low, dividend_high)
{
    let dividend = dividend_high * BIGINT_DDIGIT_BASE + dividend_low;
    let q = UINT32(dividend / divisor);
    let r = UINT32(dividend % divisor);
    return ({q: q, r: r});
}

function BigInt_ddigit_qhat_too_big(vx, qhat, ux, rhat)
{
    return (qhat >= BIGINT_DDIGIT_BASE || qhat * vx > rhat * BIGINT_DDIGIT_BASE + ux);
}

function BigInt_ddigit_multiply_sub_into(c, x, y1, y2, dst, idx)
{
    let p = y1 * y2;
    let ph = UINT32(p / BIGINT_DDIGIT_BASE);
    let pl = UINT32(p % BIGINT_DDIGIT_BASE);
    let z = x - pl - c;
    dst[idx] = z;
    return ph - (z >> BIGINT_DDIGIT_BITS);
}

function BigInt_ddigit_add_into(c, x, y, dst, idx)
{
    let z = x + y + c;
    dst[idx] = z;
    return UINT32(z / BIGINT_DDIGIT_BASE);
}

// The following functions implement unsigned arbitrary-precision
// arithmetic using the "classical" algorithms described by Knuth in
// TAOCP vol. 2, _Seminumerical Algorithms_. The division function is
// based on Henry Warren's implementation of Algorithm D in _Hacker's
// Delight_.

function BigIntSumUnsigned(x, y, sign=0)
{
    let ms = std_Math_max(BigIntSize(x), BigIntSize(y));
    let arr = new Uint32Array(ms+1);
    let c = 0;
    for (let i = 0; i < ms; i++) {
        let xi = BigIntDigit(x, i);
        let yi = BigIntDigit(y, i);
        c = BigInt_adigit_add_into(c, xi, yi, arr, i);
    }
    arr[ms] = c;
    return MakeBigInt(sign, ms + 1, arr);
}

function BigIntSubUnsigned(x, y)
{
    let sign = 0;
    if (BigIntCompareUnsigned(x, y) < 0) {
        sign = 1;
        var {y: x, x: y} = {y: y, x: x};
    }
    let ms = std_Math_max(BigIntSize(x), BigIntSize(y));
    let arr = new Uint32Array(ms);
    let c = 0;
    for (let i = 0; i < ms; i++) {
        let xi = BigIntDigit(x, i);
        let yi = BigIntDigit(y, i);
        c = BigInt_adigit_sub_into(c, xi, yi, arr, i);
    }
    return MakeBigInt(sign, ms, arr);
}

function BigIntMulUnsigned(x, y, sign=0)
{
    let m0 = BigIntSize(x);
    let n0 = BigIntSize(y);
    let m = 2*m0;
    let n = 2*n0;
    let arr = new Uint16Array(m+n);
    for (let j = 0; j < n; j++) {
        let yj = BigIntHalfDigit(y, j);
        let k = 0;
        for (let i = 0; i < m; i++) {
            let xi = BigIntHalfDigit(x, i);
            k = BigInt_mdigit_multiply_add_into(k, arr[i+j], xi, yj, arr, i+j);
        }
        arr[j + m] = k;
    }
    return MakeBigInt(sign, m0+n0, arr);
}

function BigIntDivUnsigned(x, y, sign=0)
{
    return BigIntDivModUnsigned(x, y, sign, true);
}

function BigIntModUnsigned(x, y, sign=0)
{
    return BigIntDivModUnsigned(x, y, sign, false);
}

function BigIntDivModUnsigned(x, y, sign, is_division)
{
    let n = BigIntSize(y) * 2;
    while (BigIntHalfDigit(y, n-1) == 0) {
        n--;
    }
    let m = BigIntSize(x) * 2 - n;
    let s = 0;
    while (BigIntHalfDigit(y, n-1) << s < BIGINT_DDIGIT_BASE/2) {
        s++;
    }

    let xa = new Uint16Array((BigIntSize(x) + BigIntSize(y) + 1) * 2);
    let ya = new Uint16Array(n);
    let ns = BigIntSize(x) - BigIntSize(y) + 1;
    if (ns < 1) ns = 1;
    let qa = new Uint16Array(ns*2);

    for (let i = 0; i <= n + m; i++) {
        xa[i] = BigIntHalfDigit(x, i) << s | BigIntHalfDigit(x, i-1) >> (BIGINT_DDIGIT_BITS - s);
    }
    for (let i = 0; i < n; i++) {
        ya[i] = BigIntHalfDigit(y, i) << s | BigIntHalfDigit(y, i-1) >> (BIGINT_DDIGIT_BITS - s);
    }

    for (let j = m; j >= 0; j--) {
        let {q: qhat, r: rhat} = BigInt_ddigit_divide(ya[n-1], xa[j+n-1], xa[j+n]);
        do {
            if (BigInt_ddigit_qhat_too_big(ya[n-2], qhat, xa[j+n-2], rhat)) {
                qhat--;
                rhat += ya[n-1];
                if (rhat < BIGINT_DDIGIT_BASE)
                    continue;
            }
        } while (false)
        let c = 0;
        for (let i = 0; i < n; i++) {
            c = BigInt_ddigit_multiply_sub_into(c, xa[i+j], qhat, ya[i], xa, i+j);
        }
        c = BigInt_ddigit_multiply_sub_into(c, xa[j+n], 0, 0, xa, j+n);
        qa[j] = qhat;
        if (c) {
            qa[j]--;
            let c = 0;
            for (let i = 0; i < n; i++) {
                c = BigInt_ddigit_add_into(c, xa[i+j], ya[i], xa, i+j);
            }
            BigInt_ddigit_add_into(c, xa[j+n], 0, xa, j+n);
        }
    }

    if (is_division) {
        return MakeBigInt(sign, ns, qa);
    } else {
        for (let i = 0; i < n + m; i++) {
            xa[i] = (xa[i] >> s) | (xa[i+1] << (BIGINT_DDIGIT_BITS - s));
        }
        xa[n+m] = 0;
        return MakeBigInt(sign, xa.length >> 1, xa);
    }
}

function BigIntPowUnsigned(x, n)
{
    let one = MakeOneBigInt();
    let r = one;
    for (let i = MakeZeroBigInt(); i < n; i += one)
    {
        r *= x;
    }
    return r;
}

function BigIntConstructor(x)
{
    if (typeof x === "boolean") {
        return x ? MakeOneBigInt() : MakeZeroBigInt();
    } else if (typeof x === "string") {
        return StringToBigInt(x);
    } else if (typeof x === "number") {
        return NumberToBigInt(x);
    } else {
        return MakeZeroBigInt();
    }
}

// Conversions between Number and BigInt values

function NumberToBigInt(x)
{
    let i = std_Math_floor(std_Math_abs(x));
    if (!Number_isFinite(x) || std_Math_floor(x) !== x || i > 2**53 - 1) {
        ThrowRangeError(JSMSG_NUMBER_TO_BIGINT);
    }
    let sign = x < 0 ? 1 : 0;
    if (i < 2**32) {
        let a = new Uint32Array(1);
        a[0] = i;
        return MakeBigInt(sign, 1, a);
    } else {
        let a = new Uint32Array(2);
        a[0] = i % 2**32;
        a[1] = i / 2**32;
        return MakeBigInt(sign, 2, a);
    }
}

function BigIntNumberValue(x)
{
    let sign = BigIntSign(x);
    let n = BigIntSize(x);

    if (BigIntIsZero(x)) {
        return 0;
    } else if (n > 32) {
        return (sign ? -1/0 : 1/0);
    }

    let lastDigit = BigIntDigit(x, n-1);
    let topBit = 31;
    while (topBit > 0 && lastDigit >> topBit === 0) {
        topBit--;
    }
    let expt = 32 * (n - 1) + topBit;
    lastDigit &= ~(1 << topBit);
    let significand1 = 0;
    let significand0 = 0;
    if (n === 1) {
        let lastDigitShifted = lastDigit << (32 - topBit);
        significand1 = lastDigitShifted >>> 12;
        significand0 = (lastDigitShifted & ((1 << 12) - 1)) << 20;
    } else {
        let s = 32 - topBit;
        let penultimateDigit = BigIntDigit(x, n-2);
        let lastDigitShifted = (lastDigit << s) | (penultimateDigit >>> (32 - s));
        let penultimateDigitShifted = penultimateDigit << s;
        significand1 = lastDigitShifted >>> 12;
        significand0 = (penultimateDigitShifted >>> 12) | (lastDigitShifted << 20);
    }

    let da = new Float64Array(1);
    let u8a = new Uint8Array(da.buffer);
    let u32a = new Uint32Array(da.buffer);
    if (sign)
        u8a[7] |= 0x80;
    let offsetExpt = expt + 1023;
    u32a[1] |= offsetExpt << 20;
    u32a[1] = u32a[1] | significand1;
    u32a[0] = significand0;
    return da[0];
}

// Rational representations of doubles are used for mixed-type
// comparisons.
function NumberToRational(x)
{
    let da = new Float64Array([x]);
    let u8a = new Uint8Array(da.buffer);
    let u32a = new Uint32Array(da.buffer);
    let sign = u8a[7] >> 7;
    let expt = (((u8a[7] << 4) | (u8a[6] >> 4)) & 0x7ff) - 1023;
    let siga = new Uint32Array(2);
    siga[0] = u32a[0];
    siga[1] = ((u32a[1] & 0xfffff) | 0x100000) >>> 0;
    let num = MakeBigInt(sign, 2, siga);
    let denom = BigIntLsh(MakeOneBigInt(), MakeSingleDigitBigInt(52));
    let exptBigInt = BigIntLsh(MakeOneBigInt(), MakeSingleDigitBigInt(std_Math_abs(expt)));
    if (expt < 0) {
        return [num, denom * exptBigInt];
    } else {
        return [num * exptBigInt, denom];
    }
}

function StringToBigInt(s)
{
    s = ToString(s);
    s = String_static_trim(s);
    var sign = 1;
    var i = 0;
    if (s.length > 0 && s[0] == "+") {
        i++;
    } else if (s.length > 0 && s[0] == "-") {
        i++;
        sign = -1;
    }
    var r = 10;
    if (s.length > i + 1 && s[0] == "0") {
        if (s[1] == "x" || s[1] == "X") {
            r = 16;
            i += 2;
        } else if (s[1] == "o" || s[1] == "O") {
            r = 8;
            i += 2;
        } else if (s[1] == "b" || s[1] == "B") {
            r = 2;
            i += 2;
        }
    }

    let n = MakeZeroBigInt();
    let ten = MakeSingleDigitBigInt(r);
    for (; i < s.length; i++) {
        let digit = ParseBigIntDigit(callFunction(std_String_charCodeAt, s, i));
        if (digit === undefined || digit >= r) {
            ThrowSyntaxError(JSMSG_INTEGER_INVALID_SYNTAX);
        }
        n *= ten;
        n += MakeSingleDigitBigInt(digit);
    }
    if (sign < 0) {
        return -n;
    }
    return n;
}

function ToBigInt(x)
{
    x = ToPrimitiveHintNumber(x);
    if (typeof x === "bigint")
        return x;
    else if (typeof x === "boolean")
        return x ? MakeOneBigInt() : MakeZeroBigInt();
    else if (typeof x === "string") {
        return StringToBigInt(x);
    } else {
        ThrowTypeError(JSMSG_NOT_BIGINT);
    }
}

function BigInt_asUintN(bits, bigint)
{
    bits = ToIndex(bits);
    bigint = ToBigInt(bigint);
    let bits_bn = NumberToBigInt(bits);
    let modulus = MakeOneBigInt() << bits_bn;
    let mod = bigint % modulus;
    if (mod < 0)
        mod += modulus;
    return mod;
}

function BigInt_asIntN(bits, bigint)
{
    bits = ToIndex(bits);
    bigint = ToBigInt(bigint);
    let one = MakeOneBigInt();
    let bits_bn = NumberToBigInt(bits);
    let modulus = one << bits_bn;
    let mod = bigint % modulus;
    if (mod < 0)
        mod += modulus;
    if (bits === 0) {
        return mod;
    } else {
        let modulus_half = modulus >> one;
        return (mod >= modulus_half) ? mod - modulus : mod;
    }
}

// Parsing and printing

function ParseBigIntDigit(c) {
    if (48 <= c && c <= 57) {
        // Decimal digits
        return c - 48;
    } else if (97 <= c && c <= 122) {
        // Lowercase letters
        return c - 97 + 10;
    } else if (65 <= c && c <= 90) {
        // Uppercase letters
        return c - 65 + 10;
    } else {
        return undefined;
    }
}

function ParseBigInt(s, radix)
{
    if (radix < 2 || radix >= 37) {
        ThrowRangeError(JSMSG_BAD_RADIX);
    }
    let n = MakeZeroBigInt();
    let ten = MakeSingleDigitBigInt(radix);
    for (let i = 0; i < s.length; i++) {
        let digit = ParseBigIntDigit(callFunction(std_String_charCodeAt, s, i));
        if (digit === undefined || digit >= radix) {
            ThrowSyntaxError(JSMSG_INTEGER_INVALID_SYNTAX);
        }
        n *= ten;
        n += MakeSingleDigitBigInt(digit);
    }
    return n;
}

function BigInt_parseInt(s, radix)
{
    s = ToString(s);
    s = String_static_trimLeft(s);
    var sign = 1;
    var i = 0;
    if (s.length > 0 && s[0] == "+") {
        i++;
    } else if (s.length > 0 && s[0] == "-") {
        i++;
        sign = -1;
    }
    var r = radix << 0;
    var stripPrefix = true;
    if (r != 0 && ((r < 2) || (r > 36))) {
        ThrowSyntaxError(JSMSG_INTEGER_INVALID_SYNTAX);
    }
    if (r != 0 && r != 16) {
        stripPrefix = false;
    }
    if (r == 0) {
        r = 10;
    }
    if (stripPrefix) {
        if (s.length > i + 1 &&
            s[0] == "0" &&
            (s[1] == "x" || s[1] == "X")) {
            r = 16;
            i += 2;
        }
    }

    let n = MakeZeroBigInt();
    let ten = MakeSingleDigitBigInt(r);
    let i0 = i;
    for (; i < s.length; i++) {
        let digit = ParseBigIntDigit(callFunction(std_String_charCodeAt, s, i));
        if (digit === undefined) {
            break;
        }
        if (digit >= r) {
            break;
        }
        n *= ten;
        n += MakeSingleDigitBigInt(digit);
    }
    if (i == i0) {
        ThrowSyntaxError(JSMSG_INTEGER_INVALID_SYNTAX);
    }
    if (sign < 0) {
        return -n;
    }
    return n;
}

function BigIntDigitChars()
{
    return "0123456789abcdefghijklmnopqrstuvwxyz";
}

function BigIntToStringWithRadix(n, radix=10)
{
    let size = BigIntSize(n);
    if (radix === undefined)
        radix = 10;
    else
        radix = ToInteger(radix);
    if (radix < 2 || radix >= 37) {
        ThrowRangeError(JSMSG_BAD_RADIX);
    }
    if (size === 0) {
        return "0";
    }
    let sgn = BigIntSign(n) ? "-" : "";
    let s = "";
    let bradix = MakeSingleDigitBigInt(radix);
    while (BigIntSize(n) !== 0) {
        let r = BigIntModUnsigned(n, bradix, 0);
        s = BigIntDigitChars()[BigIntDigit(r, 0)] + s;
        n = n / bradix;
    }
    return sgn + s;
}

function BigIntToString(n)
{
    return BigIntToStringWithRadix(n, 10);
}

function BigInt_toString(radix)
{
    return BigIntToStringWithRadix(this, radix);
}

// The arguments are unused, but are reserved for ECMA-402 compliance.
function BigInt_toLocaleString(reservedOne, reservedTwo)
{
    return BigIntToString(this);
}

function BigIntToInt64Low(n)
{
    let r = BigIntTcDigit(BigIntTwosComplement(n), 0);
    return r | 0;
}

function BigIntToInt64High(n)
{
    let r = BigIntTcDigit(BigIntTwosComplement(n), 1);
    return r | 0;
}

function BigIntToUint64Low(n)
{
    let r = BigIntTcDigit(BigIntTwosComplement(n), 0);
    return r | 0;
}

function BigIntToUint64High(n)
{
    let r = BigIntTcDigit(BigIntTwosComplement(n), 1);
    return r | 0;
}

function BigIntFromInt64(n1, n2)
{
    let a = new Int32Array(2);
    a[0] = n1;
    a[1] = n2;
    return BigIntFromTwosComplement(MakeBigInt(n2 < 0 ? 1 : 0, 2, a));
}

function BigIntFromUint64(n1, n2)
{
    let a = new Int32Array(2);
    a[0] = n1;
    a[1] = n2;
    return MakeBigInt(0, 2, a);
}

function DumpBigInt(n)
{
    if (BigIntSize(n) === 0)
        return "0n";
    let s = "";
    for (let i = 0; i < BigIntSize(n); i++)
    {
        let d = BigIntDigit(n, i);
        s = callFunction(String_pad_start,
                         callContentFunction(d.toString, d, 16),
                         8, "0") + s;
    }
    return (BigIntSign(n) ? "-" : "") + "0x" + s + "n";
}
